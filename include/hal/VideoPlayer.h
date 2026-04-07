#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>
#include <memory>
#include <optional>
#include "AudioPlayer.h" // 内部复用 AudioPlayer 播放视频音频

// LVGL 头文件
#include "lvgl/lvgl.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

namespace hal
{
    class VideoPlayer
    {
    public:
        // 回调类型
        using PlaybackCallback = std::function<void()>;
        using ErrorCallback = std::function<void(const std::string& errorMsg)>;
        using FrameUpdateCallback = std::function<void()>; // 通知 UI 刷新帧

        VideoPlayer();
        ~VideoPlayer();

        // 禁止拷贝
        VideoPlayer(const VideoPlayer&) = delete;
        VideoPlayer& operator=(const VideoPlayer&) = delete;

        // --- 公开接口 ---
        // 基础播放控制
        bool play(const std::string& filePath);
        void stop();
        void pause();
        void resume();

        // 视频专属：设置 LVGL 显示区域 (x, y, width, height)
        void setDisplayArea(int x, int y, int w, int h);

        // 音量控制（控制视频内置音频）
        void setVolume(float volume);

        // 获取播放进度
        std::optional<double> getCurrentPosition() const;

        // 获取总时长
        std::optional<double> getDuration() const;

        // 跳转到指定位置
        bool seek(double positionSeconds);

        // 获取暂停状态
        bool isPaused() const;

        // 获取播放状态
        bool isPlaying() const;

        // --- 回调设置 ---
        void setOnPlaybackComplete(PlaybackCallback cb);
        void setOnError(ErrorCallback cb);
        void setOnFrameUpdate(FrameUpdateCallback cb); // 新帧就绪时调用

    private:
        // --- 内部私有方法 ---
        bool openFile(const std::string& filePath);
        void decodeLoop();                     // 视频解码线程
        void renderFrame(AVFrame* frame);      // 渲染一帧到缓冲区
        void syncToAudioClock(AVFrame* frame); // 音视频同步
        void cleanup();
        void notifyError(const std::string& msg);
        void requestFrameUpdateOnUi();
        void dispatchFrameUpdateOnUi();
        void dispatchPlaybackCompleteOnUi();
        void dispatchErrorOnUi();
        static void asyncFrameUpdateThunk(void* userData);
        static void asyncPlaybackCompleteThunk(void* userData);
        static void asyncErrorThunk(void* userData);

        // --- C++17 资源管理（自定义删除器） ---
        template <typename T> struct FFmpegDeleter
        {
            void operator()(T* ptr) const noexcept
            {
                if constexpr (std::is_same_v<T, AVFormatContext>)
                {
                    if (ptr)
                        avformat_close_input(&ptr);
                }
                else if constexpr (std::is_same_v<T, AVCodecContext>)
                {
                    if (ptr)
                        avcodec_free_context(&ptr);
                }
                else if constexpr (std::is_same_v<T, SwsContext>)
                {
                    if (ptr)
                        sws_freeContext(ptr);
                }
            }
        };

        template <typename T> using FFmpegPtr = std::unique_ptr<T, FFmpegDeleter<T>>;

        // --- 成员变量 ---
        // 状态控制
        std::atomic<bool> m_isPlaying{false};
        std::atomic<bool> m_isPaused{false};
        std::atomic<bool> m_stopRequested{false};

        // 解码线程
        std::thread m_decodeThread;

        // 回调保护
        std::mutex m_callbackMutex;
        PlaybackCallback m_onComplete;
        ErrorCallback m_onError;
        FrameUpdateCallback m_onFrameUpdate;
        std::atomic<bool> m_frameUpdatePending{false};
        std::atomic<bool> m_completePending{false};
        std::atomic<bool> m_errorPending{false};
        std::mutex m_errorMutex;
        std::string m_pendingError;

        // --- 视频相关资源 ---
        FFmpegPtr<AVFormatContext> m_formatCtx{nullptr};
        FFmpegPtr<AVCodecContext> m_videoCodecCtx{nullptr};
        FFmpegPtr<SwsContext> m_swsCtx{nullptr}; // YUV -> RGB 转换
        int m_videoStreamIndex{-1};

        // 帧缓冲区（用于存储转换后的 RGB 数据，供 LVGL 使用）
        std::unique_ptr<uint8_t[]> m_rgbBuffer{nullptr};
        int m_rgbBufferSize{0};
        std::mutex m_frameMutex; // 保护 RGB 缓冲区的读写

        // --- LVGL 显示相关 ---
        lv_obj_t* m_videoCanvas{nullptr}; // LVGL 画布控件
        int m_dispX{0}, m_dispY{0}, m_dispW{0}, m_dispH{0};

        // --- 内部音频播放（复用之前的 AudioPlayer） ---
        std::unique_ptr<AudioPlayer> m_internalAudioPlayer;
        bool m_hasAudio{false};

        // 播放进度
        std::atomic<double> m_currentPts{0.0};
    };
} // namespace hal