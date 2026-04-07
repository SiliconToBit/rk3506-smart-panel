#include "VideoPlayer.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <thread>

namespace
{
    int get_online_cpus()
    {
        const long online = sysconf(_SC_NPROCESSORS_ONLN);
        return online > 0 ? static_cast<int>(online) : 1;
    }

    int select_video_cpu()
    {
        const int onlineCpus = get_online_cpus();
        if (onlineCpus >= 3)
        {
            return 2;
        }
        if (onlineCpus >= 2)
        {
            return 0;
        }
        return 0;
    }

    void bind_thread_to_cpu_if_possible(std::thread& thread, int cpuId, const char* threadName)
    {
        if (!thread.joinable())
        {
            return;
        }

        const int onlineCpus = get_online_cpus();
        if (cpuId < 0 || cpuId >= onlineCpus)
        {
            return;
        }

        cpu_set_t set;
        CPU_ZERO(&set);
        CPU_SET(cpuId, &set);

        const int ret = pthread_setaffinity_np(thread.native_handle(), sizeof(set), &set);
        if (ret != 0)
        {
            std::cerr << "[CPU] Failed to bind " << threadName << " to CPU " << cpuId << ": " << ret << '\n';
        }
    }
} // namespace

namespace hal
{

    // ==========================================
    // 构造与析构
    // ==========================================
    VideoPlayer::VideoPlayer()
        : m_internalAudioPlayer(std::make_unique<AudioPlayer>())
    {
    }

    VideoPlayer::~VideoPlayer()
    {
        stop();
    }

    // ==========================================
    // 公开接口实现
    // ==========================================
    bool VideoPlayer::play(const std::string& filePath)
    {
        if (m_isPlaying)
        {
            stop();
        }

        if (!openFile(filePath))
        {
            notifyError("Failed to open video file");
            return false;
        }

        m_stopRequested = false;
        m_isPaused = false;
        m_isPlaying = true;

        // 启动视频解码线程
        m_decodeThread = std::thread(&VideoPlayer::decodeLoop, this);
        bind_thread_to_cpu_if_possible(m_decodeThread, select_video_cpu(), "VideoDecode");

        // 如果有音频，启动音频播放
        if (m_hasAudio)
        {
            m_internalAudioPlayer->play(filePath);
        }

        return true;
    }

    void VideoPlayer::stop()
    {
        m_stopRequested = true;
        m_isPaused = false;

        if (m_decodeThread.joinable())
        {
            m_decodeThread.join();
        }

        // 停止音频
        if (m_hasAudio)
        {
            m_internalAudioPlayer->stop();
        }

        lv_async_call_cancel(asyncFrameUpdateThunk, this);
        lv_async_call_cancel(asyncPlaybackCompleteThunk, this);
        lv_async_call_cancel(asyncErrorThunk, this);
        m_frameUpdatePending = false;
        m_completePending = false;
        m_errorPending = false;

        cleanup();
        m_isPlaying = false;
    }

    void VideoPlayer::pause()
    {
        if (m_isPlaying && !m_isPaused)
        {
            m_isPaused = true;
            if (m_hasAudio)
            {
                m_internalAudioPlayer->pause();
            }
        }
    }

    void VideoPlayer::resume()
    {
        if (m_isPaused)
        {
            m_isPaused = false;
            if (m_hasAudio)
            {
                m_internalAudioPlayer->resume();
            }
        }
    }

    void VideoPlayer::setDisplayArea(int x, int y, int w, int h)
    {
        m_dispX = x;
        m_dispY = y;
        m_dispW = w;
        m_dispH = h;
    }

    void VideoPlayer::setVolume(float volume)
    {
        if (m_hasAudio)
        {
            m_internalAudioPlayer->setVolume(std::clamp(volume, 0.0F, 1.0F));
        }
    }

    std::optional<double> VideoPlayer::getCurrentPosition() const
    {
        if (m_isPlaying)
        {
            return m_currentPts.load();
        }
        return std::nullopt;
    }

    std::optional<double> VideoPlayer::getDuration() const
    {
        if (m_formatCtx && m_videoStreamIndex >= 0)
        {
            AVStream* stream = m_formatCtx->streams[m_videoStreamIndex];
            if (stream->duration != AV_NOPTS_VALUE)
            {
                return stream->duration * av_q2d(stream->time_base);
            }
            if (m_formatCtx->duration != AV_NOPTS_VALUE)
            {
                return static_cast<double>(m_formatCtx->duration) / AV_TIME_BASE;
            }
        }
        return std::nullopt;
    }

    bool VideoPlayer::seek(double positionSeconds)
    {
        if (!m_isPlaying || !m_formatCtx || m_videoStreamIndex < 0)
        {
            return false;
        }

        AVStream* stream = m_formatCtx->streams[m_videoStreamIndex];
        int64_t timestamp = static_cast<int64_t>(positionSeconds / av_q2d(stream->time_base));

        if (av_seek_frame(m_formatCtx.get(), m_videoStreamIndex, timestamp, AVSEEK_FLAG_BACKWARD) < 0)
        {
            notifyError("Video seek failed");
            return false;
        }

        avcodec_flush_buffers(m_videoCodecCtx.get());
        m_currentPts = positionSeconds;

        // 同步音频 seek
        if (m_hasAudio)
        {
            m_internalAudioPlayer->seek(positionSeconds);
        }

        return true;
    }

    bool VideoPlayer::isPaused() const
    {
        return m_isPaused.load();
    }

    bool VideoPlayer::isPlaying() const
    {
        return m_isPlaying.load();
    }

    void VideoPlayer::setOnPlaybackComplete(PlaybackCallback cb)
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        m_onComplete = std::move(cb);
    }

    void VideoPlayer::setOnError(ErrorCallback cb)
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        m_onError = std::move(cb);
    }

    void VideoPlayer::setOnFrameUpdate(FrameUpdateCallback cb)
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        m_onFrameUpdate = std::move(cb);
    }

    // ==========================================
    // 内部私有方法实现
    // ==========================================
    bool VideoPlayer::openFile(const std::string& filePath)
    {
        // 1. 打开封装格式
        AVFormatContext* rawFmtCtx = nullptr;
        if (avformat_open_input(&rawFmtCtx, filePath.c_str(), nullptr, nullptr) != 0)
        {
            return false;
        }
        m_formatCtx.reset(rawFmtCtx);

        // 2. 查找流信息
        if (avformat_find_stream_info(m_formatCtx.get(), nullptr) < 0)
        {
            return false;
        }

        // 3. 查找视频流
        m_videoStreamIndex = -1;
        for (unsigned int i = 0; i < m_formatCtx->nb_streams; i++)
        {
            if (m_formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            {
                m_videoStreamIndex = i;
                break;
            }
        }
        if (m_videoStreamIndex == -1)
        {
            return false;
        }

        // 4. 初始化视频解码器
        AVCodecParameters* codecParams = m_formatCtx->streams[m_videoStreamIndex]->codecpar;
        const AVCodec* codec = avcodec_find_decoder(codecParams->codec_id);
        if (!codec)
        {
            return false;
        }

        AVCodecContext* rawCodecCtx = avcodec_alloc_context3(codec);
        if (!rawCodecCtx)
        {
            return false;
        }
        m_videoCodecCtx.reset(rawCodecCtx);

        if (avcodec_parameters_to_context(m_videoCodecCtx.get(), codecParams) < 0)
        {
            return false;
        }
        if (avcodec_open2(m_videoCodecCtx.get(), codec, nullptr) < 0)
        {
            return false;
        }

        // 5. 初始化 YUV -> RGB 转换器
        SwsContext* rawSwsCtx =
            sws_getContext(m_videoCodecCtx->width, m_videoCodecCtx->height, m_videoCodecCtx->pix_fmt, m_dispW, m_dispH,
                           AV_PIX_FMT_RGB565LE, // LVGL 使用 RGB565
                           SWS_BILINEAR, nullptr, nullptr, nullptr);
        if (!rawSwsCtx)
        {
            return false;
        }
        m_swsCtx.reset(rawSwsCtx);

        // 6. 分配 RGB 缓冲区
        m_rgbBufferSize = m_dispW * m_dispH * 2; // RGB565 = 2 bytes per pixel
        m_rgbBuffer.reset(new uint8_t[m_rgbBufferSize]);

        // 7. 检查是否有音频流
        m_hasAudio = false;
        for (unsigned int i = 0; i < m_formatCtx->nb_streams; i++)
        {
            if (m_formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
            {
                m_hasAudio = true;
                break;
            }
        }

        return true;
    }

    void VideoPlayer::decodeLoop()
    {
        auto packetDeleter = [](AVPacket* p)
        {
            if (p)
                av_packet_free(&p);
        };
        auto frameDeleter = [](AVFrame* f)
        {
            if (f)
                av_frame_free(&f);
        };

        std::unique_ptr<AVPacket, decltype(packetDeleter)> packet(av_packet_alloc(), packetDeleter);
        std::unique_ptr<AVFrame, decltype(frameDeleter)> frame(av_frame_alloc(), frameDeleter);

        if (!packet || !frame)
        {
            notifyError("Failed to allocate AVPacket/AVFrame");
            m_isPlaying = false;
            return;
        }

        while (!m_stopRequested)
        {
            if (m_isPaused)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            // 读取一帧
            if (av_read_frame(m_formatCtx.get(), packet.get()) < 0)
            {
                break; // 播放结束
            }

            // 只处理视频流
            if (packet->stream_index != m_videoStreamIndex)
            {
                av_packet_unref(packet.get());
                continue;
            }

            // 发送包到解码器
            if (avcodec_send_packet(m_videoCodecCtx.get(), packet.get()) != 0)
            {
                av_packet_unref(packet.get());
                continue;
            }

            // 接收解码后的帧
            while (avcodec_receive_frame(m_videoCodecCtx.get(), frame.get()) == 0)
            {
                // 音视频同步
                syncToAudioClock(frame.get());

                // 将解码帧转换到本地缓冲区
                renderFrame(frame.get());

                // 更新 PTS
                if (frame->pts != AV_NOPTS_VALUE)
                {
                    AVRational timeBase = m_formatCtx->streams[m_videoStreamIndex]->time_base;
                    m_currentPts = frame->pts * av_q2d(timeBase);
                }

                // 将 UI 刷新投递回 UI 线程
                requestFrameUpdateOnUi();
            }

            av_packet_unref(packet.get());
        }

        // 自然播放完成时回调 UI 线程
        if (!m_stopRequested.load() && !m_completePending.exchange(true))
        {
            if (lv_async_call(asyncPlaybackCompleteThunk, this) != LV_RES_OK)
            {
                m_completePending = false;
            }
        }

        m_isPlaying = false;
    }

    void VideoPlayer::renderFrame(AVFrame* frame)
    {
        if (!m_rgbBuffer || !m_swsCtx)
        {
            return;
        }

        std::lock_guard<std::mutex> lock(m_frameMutex);

        // YUV -> RGB565 转换
        uint8_t* dst[1] = {m_rgbBuffer.get()};
        int dstLinesize[1] = {m_dispW * 2};

        sws_scale(m_swsCtx.get(), frame->data, frame->linesize, 0, m_videoCodecCtx->height, dst, dstLinesize);
    }

    void VideoPlayer::syncToAudioClock(AVFrame* frame)
    {
        if (!m_hasAudio)
        {
            // 无音频时，使用固定帧率同步
            AVRational frameRate = m_formatCtx->streams[m_videoStreamIndex]->avg_frame_rate;
            if (frameRate.num > 0 && frameRate.den > 0)
            {
                double frameDuration = av_q2d(av_inv_q(frameRate));
                std::this_thread::sleep_for(std::chrono::duration<double>(frameDuration));
            }
            return;
        }

        // 有音频时，同步到音频时钟
        auto audioPos = m_internalAudioPlayer->getCurrentPosition();
        if (audioPos)
        {
            double videoPts = 0.0;
            if (frame->pts != AV_NOPTS_VALUE)
            {
                AVRational timeBase = m_formatCtx->streams[m_videoStreamIndex]->time_base;
                videoPts = frame->pts * av_q2d(timeBase);
            }

            double diff = videoPts - audioPos.value();
            if (diff > 0.02) // 视频超前，等待
            {
                std::this_thread::sleep_for(std::chrono::duration<double>(diff));
            }
            else if (diff < -0.1) // 视频落后太多，丢帧
            {
                // 不渲染，直接跳过
            }
        }
    }

    void VideoPlayer::cleanup()
    {
        m_videoStreamIndex = -1;
        m_hasAudio = false;
        m_currentPts = 0.0;
        m_rgbBuffer.reset();
        m_rgbBufferSize = 0;
    }

    void VideoPlayer::notifyError(const std::string& msg)
    {
        {
            std::lock_guard<std::mutex> lock(m_errorMutex);
            m_pendingError = msg;
        }

        if (m_errorPending.exchange(true))
        {
            return;
        }

        if (lv_async_call(asyncErrorThunk, this) != LV_RES_OK)
        {
            m_errorPending = false;
            std::cerr << "[VideoPlayer Error] " << msg << '\n';
        }
    }

    void VideoPlayer::requestFrameUpdateOnUi()
    {
        if (m_frameUpdatePending.exchange(true))
        {
            return;
        }

        if (lv_async_call(asyncFrameUpdateThunk, this) != LV_RES_OK)
        {
            m_frameUpdatePending = false;
        }
    }

    void VideoPlayer::dispatchFrameUpdateOnUi()
    {
        m_frameUpdatePending = false;

        if (m_videoCanvas && m_rgbBuffer && m_dispW > 0 && m_dispH > 0)
        {
            std::lock_guard<std::mutex> lock(m_frameMutex);
            lv_canvas_set_buffer(m_videoCanvas, m_rgbBuffer.get(), m_dispW, m_dispH, LV_IMG_CF_TRUE_COLOR);
            lv_obj_invalidate(m_videoCanvas);
        }

        FrameUpdateCallback callback;
        {
            std::lock_guard<std::mutex> lock(m_callbackMutex);
            callback = m_onFrameUpdate;
        }

        if (callback)
        {
            callback();
        }
    }

    void VideoPlayer::dispatchPlaybackCompleteOnUi()
    {
        m_completePending = false;

        PlaybackCallback callback;
        {
            std::lock_guard<std::mutex> lock(m_callbackMutex);
            callback = m_onComplete;
        }

        if (callback)
        {
            callback();
        }
    }

    void VideoPlayer::dispatchErrorOnUi()
    {
        m_errorPending = false;

        std::string pendingError;
        {
            std::lock_guard<std::mutex> lock(m_errorMutex);
            pendingError = m_pendingError;
        }

        ErrorCallback callback;
        {
            std::lock_guard<std::mutex> lock(m_callbackMutex);
            callback = m_onError;
        }

        if (callback)
        {
            callback(pendingError);
        }
        else
        {
            std::cerr << "[VideoPlayer Error] " << pendingError << '\n';
        }
    }

    void VideoPlayer::asyncFrameUpdateThunk(void* userData)
    {
        auto* self = static_cast<VideoPlayer*>(userData);
        if (self)
        {
            self->dispatchFrameUpdateOnUi();
        }
    }

    void VideoPlayer::asyncPlaybackCompleteThunk(void* userData)
    {
        auto* self = static_cast<VideoPlayer*>(userData);
        if (self)
        {
            self->dispatchPlaybackCompleteOnUi();
        }
    }

    void VideoPlayer::asyncErrorThunk(void* userData)
    {
        auto* self = static_cast<VideoPlayer*>(userData);
        if (self)
        {
            self->dispatchErrorOnUi();
        }
    }

} // namespace hal
