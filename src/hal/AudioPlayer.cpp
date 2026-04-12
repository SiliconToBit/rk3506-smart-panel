#include "AudioPlayer.h"
#include "hal/CpuAffinity.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>

namespace hal
{
    // ==========================================
    // 构造与析构
    // ==========================================
    AudioPlayer::AudioPlayer() = default;

    AudioPlayer::~AudioPlayer()
    {
        stop(); // 析构时确保停止并清理资源
    }

    // ==========================================
    // 公开接口实现
    // ==========================================
    bool AudioPlayer::play(const std::string& filePath)
    {
        // 如果正在播放，先停止
        if (m_isPlaying)
        {
            stop();
        }

        // 1. 打开文件并初始化所有组件
        if (!openFile(filePath))
        {
            notifyError("Failed to open file or init codec");
            return false;
        }

        // 2. 重置状态
        m_stopRequested = false;
        m_isPaused = false;
        m_isPlaying = true;

        // 3. 启动解码线程 (C++11 起，std::thread 直接移动)
        m_decodeThread = std::thread(&AudioPlayer::decodeLoop, this);

        // 使用自定义CPU亲和性或默认值
        const int targetCpu = (m_cpuAffinity >= 0) ? m_cpuAffinity : selectAudioCpu();
        bindThreadToCpu(m_decodeThread, targetCpu, "AudioDecode");

        return true;
    }

    void AudioPlayer::stop()
    {
        if (!m_isPlaying)
        {
            return;
        }

        // 请求停止
        m_stopRequested = true;
        m_isPaused = false; // 唤醒可能的暂停等待

        // 等待线程结束
        if (m_decodeThread.joinable())
        {
            m_decodeThread.join();
        }

        // 清理资源
        cleanup();
    }

    void AudioPlayer::pause()
    {
        if (m_isPlaying && !m_isPaused)
        {
            m_isPaused = true;
        }
    }

    void AudioPlayer::resume()
    {
        if (m_isPaused)
        {
            m_isPaused = false;
        }
    }

    void AudioPlayer::setVolume(float volume)
    {
        m_volume = std::clamp(volume, 0.0F, 1.0F); // C++17 clamp
    }

    std::optional<double> AudioPlayer::getCurrentPosition() const
    {
        if (m_isPlaying)
        {
            return m_currentPts.load();
        }
        return std::nullopt; // C++17 表示无值
    }

    std::optional<double> AudioPlayer::getDuration() const
    {
        if (m_formatCtx && m_audioStreamIndex >= 0)
        {
            AVStream* stream = m_formatCtx->streams[m_audioStreamIndex];
            if (stream->duration != AV_NOPTS_VALUE)
            {
                return stream->duration * av_q2d(stream->time_base);
            }
            // 如果 duration 不可用，尝试从封装格式获取
            if (m_formatCtx->duration != AV_NOPTS_VALUE)
            {
                return static_cast<double>(m_formatCtx->duration) / AV_TIME_BASE;
            }
        }
        return std::nullopt;
    }

    bool AudioPlayer::seek(double positionSeconds)
    {
        if (!m_isPlaying || !m_formatCtx || m_audioStreamIndex < 0)
        {
            return false;
        }

        // 将秒转换为时间戳
        AVStream* stream = m_formatCtx->streams[m_audioStreamIndex];
        int64_t timestamp = static_cast<int64_t>(positionSeconds / av_q2d(stream->time_base));

        // 执行 seek
        if (av_seek_frame(m_formatCtx.get(), m_audioStreamIndex, timestamp, AVSEEK_FLAG_BACKWARD) < 0)
        {
            notifyError("Seek failed");
            return false;
        }

        // 刷新解码器缓冲区
        avcodec_flush_buffers(m_codecCtx.get());

        // 更新当前 PTS
        m_currentPts = positionSeconds;

        return true;
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

    // ==========================================
    // 内部私有方法实现
    // ==========================================
    bool AudioPlayer::openFile(const std::string& filePath)
    {
        // 1. 打开封装格式上下文
        AVFormatContext* rawFmtCtx = nullptr;
        if (avformat_open_input(&rawFmtCtx, filePath.c_str(), nullptr, nullptr) != 0)
        {
            return false;
        }
        m_formatCtx.reset(rawFmtCtx); // 交给智能指针管理

        // 2. 查找流信息
        if (avformat_find_stream_info(m_formatCtx.get(), nullptr) < 0)
        {
            return false;
        }

        // 3. 查找音频流
        for (unsigned int i = 0; i < m_formatCtx->nb_streams; i++)
        {
            if (m_formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
            {
                m_audioStreamIndex = i;
                break;
            }
        }
        if (m_audioStreamIndex == -1)
        {
            return false;
        }

        // 4. 初始化解码器
        AVCodecParameters* codecParams = m_formatCtx->streams[m_audioStreamIndex]->codecpar;
        const AVCodec* codec = avcodec_find_decoder(codecParams->codec_id);
        if (codec == nullptr)
        {
            return false;
        }

        AVCodecContext* rawCodecCtx = avcodec_alloc_context3(codec);
        if (rawCodecCtx == nullptr)
        {
            return false;
        }
        m_codecCtx.reset(rawCodecCtx);

        if (avcodec_parameters_to_context(m_codecCtx.get(), codecParams) < 0)
        {
            return false;
        }
        if (avcodec_open2(m_codecCtx.get(), codec, nullptr) < 0)
        {
            return false;
        }

        // 5. 初始化重采样器 (SwrContext) - 转为 ALSA 需要的格式 (通常是 S16, 44.1k, 双声道)
        SwrContext* rawSwrCtx = swr_alloc();
        if (rawSwrCtx == nullptr)
        {
            return false;
        }
        m_swrCtx.reset(rawSwrCtx);

        // 输入参数
        av_opt_set_channel_layout(m_swrCtx.get(), "in_channel_layout", m_codecCtx->channel_layout, 0);
        av_opt_set_int(m_swrCtx.get(), "in_sample_rate", m_codecCtx->sample_rate, 0);
        av_opt_set_sample_fmt(m_swrCtx.get(), "in_sample_fmt", m_codecCtx->sample_fmt, 0);

        // 输出参数 (目标: 16位深度, 立体声, 44100Hz)
        const int outChannels = kOutputChannels;
        const int outSampleRate = kOutputSampleRate;
        const AVSampleFormat outFmt = AV_SAMPLE_FMT_S16;

        av_opt_set_channel_layout(m_swrCtx.get(), "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
        av_opt_set_int(m_swrCtx.get(), "out_sample_rate", outSampleRate, 0);
        av_opt_set_sample_fmt(m_swrCtx.get(), "out_sample_fmt", outFmt, 0);

        if (swr_init(m_swrCtx.get()) < 0)
        {
            return false;
        }

        // 6. 初始化 ALSA
        snd_pcm_t* rawPcmHandle = nullptr;
        if (snd_pcm_open(&rawPcmHandle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0)
        {
            std::cerr << "ALSA open failed\n";
            return false;
        }
        m_pcmHandle.reset(rawPcmHandle);

        // 配置 ALSA 硬件参数
        snd_pcm_hw_params_t* params;
        snd_pcm_hw_params_alloca(&params);
        snd_pcm_hw_params_any(m_pcmHandle.get(), params);
        snd_pcm_hw_params_set_access(m_pcmHandle.get(), params, SND_PCM_ACCESS_RW_INTERLEAVED);
        snd_pcm_hw_params_set_format(m_pcmHandle.get(), params, SND_PCM_FORMAT_S16_LE);
        snd_pcm_hw_params_set_channels(m_pcmHandle.get(), params, outChannels);
        unsigned int rate = outSampleRate;
        snd_pcm_hw_params_set_rate_near(m_pcmHandle.get(), params, &rate, nullptr);
        snd_pcm_hw_params(m_pcmHandle.get(), params);
        snd_pcm_prepare(m_pcmHandle.get());

        return true;
    }

    int AudioPlayer::convertFrameSamples(AVFrame* frame, uint8_t*& outBuffer, int outSamples) const
    {
        return swr_convert(m_swrCtx.get(), &outBuffer, outSamples, (const uint8_t**) frame->data, frame->nb_samples);
    }

    void AudioPlayer::applySoftwareVolume(uint8_t* outBuffer, int sampleCount) const
    {
        const float volume = m_volume.load();
        if (volume >= 1.0F)
        {
            return;
        }

        auto* samples = reinterpret_cast<int16_t*>(outBuffer);
        const int totalSamples = sampleCount * kOutputChannels;
        for (int i = 0; i < totalSamples; ++i)
        {
            samples[i] = static_cast<int16_t>(samples[i] * volume);
        }
    }

    void AudioPlayer::writePcmSamples(uint8_t* outBuffer, int sampleCount)
    {
        snd_pcm_sframes_t framesWritten = snd_pcm_writei(m_pcmHandle.get(), outBuffer, sampleCount);
        if (framesWritten < 0)
        {
            snd_pcm_recover(m_pcmHandle.get(), framesWritten, 1);
        }
    }

    void AudioPlayer::updatePlaybackPosition(const AVFrame* frame)
    {
        if (frame->pts == AV_NOPTS_VALUE)
        {
            return;
        }

        const AVRational timeBase = m_formatCtx->streams[m_audioStreamIndex]->time_base;
        m_currentPts = frame->pts * av_q2d(timeBase);
    }

    void AudioPlayer::processAudioPacket(const AVPacket* packet, AVFrame* frame, uint8_t* outBuffer, int outSamples)
    {
        if (packet->stream_index != m_audioStreamIndex)
        {
            return;
        }

        if (avcodec_send_packet(m_codecCtx.get(), packet) != 0)
        {
            return;
        }

        while (avcodec_receive_frame(m_codecCtx.get(), frame) == 0)
        {
            uint8_t* outBufPtr = outBuffer;
            const int outSamplesCount = convertFrameSamples(frame, outBufPtr, outSamples);

            if (outSamplesCount <= 0)
            {
                continue;
            }

            applySoftwareVolume(outBufPtr, outSamplesCount);
            writePcmSamples(outBufPtr, outSamplesCount);
            updatePlaybackPosition(frame);
        }
    }

    void AudioPlayer::decodeLoop()
    {
        auto packetDeleter = [](AVPacket* packetPtr)
        {
            if (packetPtr)
            {
                av_packet_free(&packetPtr);
            }
        };
        auto frameDeleter = [](AVFrame* framePtr)
        {
            if (framePtr)
            {
                av_frame_free(&framePtr);
            }
        };

        std::unique_ptr<AVPacket, decltype(packetDeleter)> packet(av_packet_alloc(), packetDeleter);
        std::unique_ptr<AVFrame, decltype(frameDeleter)> frame(av_frame_alloc(), frameDeleter);

        if (!packet || !frame)
        {
            notifyError("Failed to allocate AVPacket/AVFrame");
            m_isPlaying = false;
            return;
        }

        // 输出缓冲区
        const int outSamples = kOutputSampleRate * kOutputChannels; // 1秒缓冲
        std::unique_ptr<uint8_t[]> outBuffer(new uint8_t[outSamples * kOutputChannels * sizeof(int16_t)]);

        while (!m_stopRequested)
        {
            // 暂停处理
            if (m_isPaused)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            // 1. 读取一帧数据
            if (av_read_frame(m_formatCtx.get(), packet.get()) < 0)
            {
                break; // 读完了或出错
            }

            processAudioPacket(packet.get(), frame.get(), outBuffer.get(), outSamples);
            av_packet_unref(packet.get());
        }

        // 播放完成回调
        {
            std::lock_guard<std::mutex> lock(m_callbackMutex);
            if (m_onComplete)
            {
                m_onComplete();
            }
        }

        m_isPlaying = false;
    }

    void AudioPlayer::cleanup()
    {
        // 智能指针会自动释放 m_formatCtx, m_codecCtx, m_swrCtx, m_pcmHandle

        m_audioStreamIndex = -1;
        m_currentPts = 0.0;
    }

    void AudioPlayer::notifyError(const std::string& msg)
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        if (m_onError)
        {
            m_onError(msg);
        }
    }
} // namespace hal