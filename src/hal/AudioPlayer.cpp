#include "AudioPlayer.h"

#include "hal/AudioDecoder.h"
#include "hal/AudioOutput.h"
#include "hal/CpuAffinity.h"

#include <algorithm>
#include <chrono>
#include <utility>
#include <vector>

namespace hal
{
    AudioPlayer::AudioPlayer()
        : m_decoder(std::make_unique<AudioDecoder>())
        , m_output(std::make_unique<AudioOutput>())
    {
        m_callbackThread = std::thread(&AudioPlayer::callbackLoop, this);
        m_controlThread = std::thread(&AudioPlayer::controlLoop, this);
    }

    AudioPlayer::~AudioPlayer()
    {
        {
            std::lock_guard<std::mutex> lock(m_commandMutex);
            m_exitRequested = true;
            m_commandQueue.push_back(Command{CommandType::Shutdown, "", 0.0, nullptr});
        }
        m_commandCv.notify_one();

        if (m_controlThread.joinable())
        {
            m_controlThread.join();
        }

        {
            std::lock_guard<std::mutex> lock(m_callbackQueueMutex);
            m_callbackExitRequested = true;
        }
        m_callbackQueueCv.notify_one();

        if (m_callbackThread.joinable())
        {
            m_callbackThread.join();
        }
    }

    bool AudioPlayer::play(const std::string& filePath)
    {
        if (isOnControlThread())
        {
            return handlePlayCommand(filePath);
        }

        bool result = false;
        if (!enqueueAndWait(Command{CommandType::Play, filePath, 0.0, nullptr}, result))
        {
            notifyError("Play command timeout");
            return false;
        }
        return result;
    }

    void AudioPlayer::stop()
    {
        if (isOnControlThread())
        {
            handleStopCommand();
            return;
        }

        bool result = false;
        if (!enqueueAndWait(Command{CommandType::Stop, "", 0.0, nullptr}, result))
        {
            notifyError("Stop command timeout");
        }
    }

    void AudioPlayer::pause()
    {
        enqueueCommand(Command{CommandType::Pause, "", 0.0, nullptr});
    }

    void AudioPlayer::resume()
    {
        enqueueCommand(Command{CommandType::Resume, "", 0.0, nullptr});
    }

    void AudioPlayer::setVolume(float volume)
    {
        m_volume = std::clamp(volume, 0.0F, 1.0F);
    }

    std::optional<double> AudioPlayer::getCurrentPosition() const
    {
        if (m_isPlaying)
        {
            return m_currentPts.load();
        }
        return std::nullopt;
    }

    std::optional<double> AudioPlayer::getDuration() const
    {
        if (m_hasDuration.load())
        {
            return m_durationSeconds.load();
        }
        return std::nullopt;
    }

    bool AudioPlayer::seek(double positionSeconds)
    {
        if (isOnControlThread())
        {
            return handleSeekCommand(positionSeconds);
        }

        bool result = false;
        if (!enqueueAndWait(Command{CommandType::Seek, "", positionSeconds, nullptr}, result))
        {
            notifyError("Seek command timeout");
            return false;
        }
        return result;
    }

    bool AudioPlayer::isPaused() const
    {
        return m_isPaused.load();
    }

    bool AudioPlayer::isPlaying() const
    {
        return m_isPlaying.load();
    }

    void AudioPlayer::setOnPlaybackComplete(PlaybackCallback callback)
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        m_onComplete = std::move(callback);
    }

    void AudioPlayer::setOnError(ErrorCallback callback)
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        m_onError = std::move(callback);
    }

    void AudioPlayer::setCpuAffinity(int cpuId)
    {
        m_cpuAffinity = cpuId;
    }

    void AudioPlayer::controlLoop()
    {
        m_controlThreadId = std::this_thread::get_id();

        while (true)
        {
            Command command;
            bool hasCommand = false;

            {
                std::unique_lock<std::mutex> lock(m_commandMutex);

                if (m_commandQueue.empty())
                {
                    if (m_isPlaying.load() && !m_isPaused.load())
                    {
                        m_commandCv.wait_for(lock, std::chrono::milliseconds(2),
                                             [this]() { return !m_commandQueue.empty(); });
                    }
                    else
                    {
                        m_commandCv.wait(lock, [this]() { return !m_commandQueue.empty(); });
                    }
                }

                if (!m_commandQueue.empty())
                {
                    command = std::move(m_commandQueue.front());
                    m_commandQueue.pop_front();
                    hasCommand = true;
                }
            }

            if (hasCommand)
            {
                processCommand(command);
                if (command.type == CommandType::Shutdown)
                {
                    break;
                }
                continue;
            }

            if (m_isPlaying.load() && !m_isPaused.load())
            {
                decodeOneStep();
            }
        }

        resetPlaybackResources();
        m_isPlaying = false;
        m_isPaused = false;
    }

    void AudioPlayer::processCommand(const Command& command)
    {
        switch (command.type)
        {
            case CommandType::Play:
            {
                const bool result = handlePlayCommand(command.filePath);
                signalSyncResult(command.sync, result);
                break;
            }
            case CommandType::Stop:
            {
                handleStopCommand();
                signalSyncResult(command.sync, true);
                break;
            }
            case CommandType::Pause:
            {
                handlePauseCommand();
                break;
            }
            case CommandType::Resume:
            {
                handleResumeCommand();
                break;
            }
            case CommandType::Seek:
            {
                const bool result = handleSeekCommand(command.seekPositionSeconds);
                signalSyncResult(command.sync, result);
                break;
            }
            case CommandType::Shutdown:
            {
                if (command.sync != nullptr)
                {
                    signalSyncResult(command.sync, true);
                }
                break;
            }
        }
    }

    void AudioPlayer::enqueueCommand(Command&& command)
    {
        {
            std::lock_guard<std::mutex> lock(m_commandMutex);
            m_commandQueue.emplace_back(std::move(command));
        }
        m_commandCv.notify_one();
    }

    bool AudioPlayer::enqueueAndWait(Command&& command, bool& result)
    {
        auto sync = std::make_shared<SyncToken>();
        command.sync = sync;
        enqueueCommand(std::move(command));

        std::unique_lock<std::mutex> lock(sync->mutex);
        if (!sync->cv.wait_for(lock, kSyncCommandTimeout, [sync]() { return sync->done; }))
        {
            result = false;
            return false;
        }

        result = sync->result;
        return true;
    }

    void AudioPlayer::signalSyncResult(const std::shared_ptr<SyncToken>& sync, bool result)
    {
        if (!sync)
        {
            return;
        }

        {
            std::lock_guard<std::mutex> lock(sync->mutex);
            sync->result = result;
            sync->done = true;
        }

        sync->cv.notify_one();
    }

    bool AudioPlayer::isOnControlThread() const
    {
        return std::this_thread::get_id() == m_controlThreadId;
    }

    bool AudioPlayer::handlePlayCommand(const std::string& filePath)
    {
        handleStopCommand();

        if (!m_decoder->open(filePath))
        {
            notifyError("Failed to open file or init decoder");
            resetPlaybackResources();
            return false;
        }

        std::string outputError;
        if (!m_output->open(AudioDecoder::kOutputSampleRate, AudioDecoder::kOutputChannels, outputError))
        {
            notifyError(outputError.empty() ? "Failed to open audio output" : outputError);
            resetPlaybackResources();
            return false;
        }

        const int targetCpu = (m_cpuAffinity >= 0) ? m_cpuAffinity : selectAudioCpu();
        bindThreadToCpu(m_controlThread, targetCpu, "AudioControl");

        const auto duration = m_decoder->getDurationSeconds();
        if (duration.has_value())
        {
            m_durationSeconds = duration.value();
            m_hasDuration = true;
        }
        else
        {
            m_durationSeconds = 0.0;
            m_hasDuration = false;
        }

        m_currentPts = 0.0;
        m_isPaused = false;
        m_isPlaying = true;
        return true;
    }

    void AudioPlayer::handleStopCommand()
    {
        resetPlaybackResources();
        m_isPlaying = false;
        m_isPaused = false;
        m_currentPts = 0.0;
        m_durationSeconds = 0.0;
        m_hasDuration = false;
    }

    void AudioPlayer::handlePauseCommand()
    {
        if (m_isPlaying.load())
        {
            m_isPaused = true;
        }
    }

    void AudioPlayer::handleResumeCommand()
    {
        if (m_isPlaying.load())
        {
            m_isPaused = false;
        }
    }

    bool AudioPlayer::handleSeekCommand(double positionSeconds)
    {
        if (!m_isPlaying.load())
        {
            return false;
        }

        if (!m_decoder->seek(positionSeconds))
        {
            notifyError("Seek failed");
            return false;
        }

        m_output->flush();
        m_currentPts = positionSeconds;
        return true;
    }

    void AudioPlayer::decodeOneStep()
    {
        std::vector<int16_t> pcmBuffer;
        std::string decodeError;
        double ptsSeconds = m_currentPts.load();

        if (!m_decoder->readNextPcm(pcmBuffer, ptsSeconds, decodeError))
        {
            const bool isNaturalComplete = decodeError.empty();
            if (!isNaturalComplete)
            {
                notifyError(decodeError);
            }

            handleStopCommand();

            if (isNaturalComplete)
            {
                notifyPlaybackComplete();
            }
            return;
        }

        if (pcmBuffer.empty())
        {
            return;
        }

        std::string outputError;
        const int frameCount = static_cast<int>(pcmBuffer.size() / AudioDecoder::kOutputChannels);
        if (!m_output->write(pcmBuffer.data(), frameCount, m_volume.load(), outputError))
        {
            notifyError(outputError.empty() ? "Audio output write failed" : outputError);
            handleStopCommand();
            return;
        }

        m_currentPts = ptsSeconds;
    }

    void AudioPlayer::resetPlaybackResources()
    {
        if (m_output)
        {
            m_output->close();
        }
        if (m_decoder)
        {
            m_decoder->close();
        }
    }

    void AudioPlayer::notifyError(const std::string& msg)
    {
        ErrorCallback onError;
        {
            std::lock_guard<std::mutex> lock(m_callbackMutex);
            onError = m_onError;
        }

        if (onError)
        {
            postCallbackTask([onError, msg]() { onError(msg); });
        }
    }

    void AudioPlayer::notifyPlaybackComplete()
    {
        PlaybackCallback onComplete;
        {
            std::lock_guard<std::mutex> lock(m_callbackMutex);
            onComplete = m_onComplete;
        }

        if (onComplete)
        {
            postCallbackTask([onComplete]() { onComplete(); });
        }
    }

    void AudioPlayer::postCallbackTask(std::function<void()> task)
    {
        {
            std::lock_guard<std::mutex> lock(m_callbackQueueMutex);
            if (m_callbackExitRequested)
            {
                return;
            }
            m_callbackQueue.emplace_back(std::move(task));
        }
        m_callbackQueueCv.notify_one();
    }

    void AudioPlayer::callbackLoop()
    {
        while (true)
        {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(m_callbackQueueMutex);
                m_callbackQueueCv.wait(lock, [this]() { return m_callbackExitRequested || !m_callbackQueue.empty(); });

                if (m_callbackExitRequested && m_callbackQueue.empty())
                {
                    break;
                }

                if (!m_callbackQueue.empty())
                {
                    task = std::move(m_callbackQueue.front());
                    m_callbackQueue.pop_front();
                }
            }

            if (task)
            {
                task();
            }
        }
    }
} // namespace hal
