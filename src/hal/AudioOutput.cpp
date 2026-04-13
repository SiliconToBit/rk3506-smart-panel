#include "hal/AudioOutput.h"

#include <algorithm>
#include <cerrno>
#include <memory>
#include <vector>

extern "C"
{
#include <alsa/asoundlib.h>
}

namespace
{
    struct PcmDeleter
    {
        void operator()(snd_pcm_t* ptr) const noexcept
        {
            if (ptr != nullptr)
            {
                // 切歌/停止时优先快速中断，避免 drain 等待缓冲播放完导致 UI 卡顿
                snd_pcm_drop(ptr);
                snd_pcm_close(ptr);
            }
        }
    };

    using PcmPtr = std::unique_ptr<snd_pcm_t, PcmDeleter>;
} // namespace

namespace hal
{
    struct AudioOutput::Impl
    {
        PcmPtr pcmHandle{nullptr};
        int channels{0};
    };

    AudioOutput::AudioOutput()
        : m_impl(std::make_unique<Impl>())
    {
    }

    AudioOutput::~AudioOutput()
    {
        close();
    }

    bool AudioOutput::open(int sampleRate, int channels, std::string& errorMsg)
    {
        errorMsg.clear();
        close();

        snd_pcm_t* rawPcmHandle = nullptr;
        // 使用非阻塞模式打开，避免设备阻塞拖慢上层命令响应
        const int openRet = snd_pcm_open(&rawPcmHandle, "default", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
        if (openRet < 0)
        {
            errorMsg = "ALSA open failed: " + std::string(snd_strerror(openRet));
            return false;
        }

        m_impl->pcmHandle.reset(rawPcmHandle);

        snd_pcm_hw_params_t* params = nullptr;
        snd_pcm_hw_params_alloca(&params);
        snd_pcm_hw_params_any(m_impl->pcmHandle.get(), params);
        snd_pcm_hw_params_set_access(m_impl->pcmHandle.get(), params, SND_PCM_ACCESS_RW_INTERLEAVED);
        snd_pcm_hw_params_set_format(m_impl->pcmHandle.get(), params, SND_PCM_FORMAT_S16_LE);
        snd_pcm_hw_params_set_channels(m_impl->pcmHandle.get(), params, channels);

        unsigned int rate = static_cast<unsigned int>(sampleRate);
        snd_pcm_hw_params_set_rate_near(m_impl->pcmHandle.get(), params, &rate, nullptr);

        const int hwRet = snd_pcm_hw_params(m_impl->pcmHandle.get(), params);
        if (hwRet < 0)
        {
            errorMsg = "ALSA hw_params failed: " + std::string(snd_strerror(hwRet));
            close();
            return false;
        }

        const int prepareRet = snd_pcm_prepare(m_impl->pcmHandle.get());
        if (prepareRet < 0)
        {
            errorMsg = "ALSA prepare failed: " + std::string(snd_strerror(prepareRet));
            close();
            return false;
        }

        m_impl->channels = channels;
        return true;
    }

    void AudioOutput::close()
    {
        m_impl->pcmHandle.reset();
        m_impl->channels = 0;
    }

    bool AudioOutput::isOpen() const
    {
        return m_impl->pcmHandle != nullptr && m_impl->channels > 0;
    }

    void AudioOutput::flush()
    {
        if (!isOpen())
        {
            return;
        }

        snd_pcm_drop(m_impl->pcmHandle.get());
        snd_pcm_prepare(m_impl->pcmHandle.get());
    }

    bool AudioOutput::write(const int16_t* pcmInterleaved, int frameCount, float volume, std::string& errorMsg)
    {
        errorMsg.clear();
        if (!isOpen() || pcmInterleaved == nullptr || frameCount <= 0)
        {
            return false;
        }

        std::vector<int16_t> scaled;
        const int16_t* outputData = pcmInterleaved;
        const float clampedVolume = std::clamp(volume, 0.0F, 1.0F);
        if (clampedVolume < 0.999F)
        {
            const size_t totalSamples = static_cast<size_t>(frameCount) * static_cast<size_t>(m_impl->channels);
            scaled.assign(pcmInterleaved, pcmInterleaved + totalSamples);
            for (auto& sample : scaled)
            {
                sample = static_cast<int16_t>(sample * clampedVolume);
            }
            outputData = scaled.data();
        }

        int writtenFrames = 0;
        int consecutiveEagainCount = 0;
        constexpr int kMaxEagainRetry = 200; // 约 4 秒（200 * 20ms）

        while (writtenFrames < frameCount)
        {
            const int remainingFrames = frameCount - writtenFrames;
            const int16_t* currentPtr = outputData + (static_cast<size_t>(writtenFrames) * m_impl->channels);
            snd_pcm_sframes_t writeRet = snd_pcm_writei(m_impl->pcmHandle.get(), currentPtr, remainingFrames);

            if (writeRet > 0)
            {
                writtenFrames += static_cast<int>(writeRet);
                consecutiveEagainCount = 0;
                continue;
            }

            if (writeRet == -EAGAIN)
            {
                ++consecutiveEagainCount;
                if (consecutiveEagainCount > kMaxEagainRetry)
                {
                    errorMsg = "ALSA write timeout (device busy)";
                    return false;
                }

                const int waitRet = snd_pcm_wait(m_impl->pcmHandle.get(), 20);
                if (waitRet < 0)
                {
                    const int recoverRet = snd_pcm_recover(m_impl->pcmHandle.get(), waitRet, 1);
                    if (recoverRet < 0)
                    {
                        errorMsg = "ALSA wait/recover failed: " + std::string(snd_strerror(recoverRet));
                        return false;
                    }
                }
                continue;
            }

            const int recoverRet = snd_pcm_recover(m_impl->pcmHandle.get(), static_cast<int>(writeRet), 1);
            if (recoverRet < 0)
            {
                errorMsg = "ALSA recover failed: " + std::string(snd_strerror(recoverRet));
                return false;
            }
        }

        return true;
    }
} // namespace hal
