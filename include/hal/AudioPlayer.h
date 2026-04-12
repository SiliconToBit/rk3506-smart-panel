#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <type_traits>

extern "C"
{
#include <alsa/asoundlib.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
}

namespace hal
{
    class AudioPlayer
    {
    public:
        // 状态回调类型定义
        using PlaybackCallback = std::function<void()>;
        using ErrorCallback = std::function<void(const std::string& errorMsg)>;

        AudioPlayer();
        ~AudioPlayer();

        // 禁止拷贝和赋值（C++11 起）
        AudioPlayer(const AudioPlayer&) = delete;
        AudioPlayer& operator=(const AudioPlayer&) = delete;

        // --- 公开接口 ---
        bool play(const std::string& filePath); // 开始播放
        void stop();                            // 停止播放
        void pause();                           // 暂停
        void resume();                          // 恢复

        // 设置音量 (0.0 - 1.0)
        void setVolume(float volume);

        // 获取当前播放进度 (秒)，C++17 使用 optional 表示可能无效
        [[nodiscard]] std::optional<double> getCurrentPosition() const;

        // 获取总时长 (秒)
        [[nodiscard]] std::optional<double> getDuration() const;

        // 跳转到指定位置 (秒)
        bool seek(double positionSeconds);

        // 获取暂停状态
        [[nodiscard]] bool isPaused() const;

        // 获取播放状态
        [[nodiscard]] bool isPlaying() const;

        // --- 回调设置 ---
        void setOnPlaybackComplete(PlaybackCallback callback);
        void setOnError(ErrorCallback callback);

        // 设置CPU亲和性（必须在play()之前调用）
        void setCpuAffinity(int cpuId);

    private:
        static constexpr int kOutputChannels = 2;
        static constexpr int kOutputSampleRate = 44100;

        // --- 内部私有方法 ---
        bool openFile(const std::string& filePath);                                         // 打开文件并初始化解码器
        void decodeLoop();                                                                  // 解码线程主循环
        int convertFrameSamples(AVFrame* frame, uint8_t*& outBuffer, int outSamples) const; // 重采样并返回输出采样数
        void applySoftwareVolume(uint8_t* outBuffer, int sampleCount) const;                // 对 PCM 数据应用软件音量
        void writePcmSamples(uint8_t* outBuffer, int sampleCount); // 写入 ALSA 并在失败时执行 recover
        void updatePlaybackPosition(const AVFrame* frame);         // 使用帧时间戳更新当前播放进度
        void processAudioPacket(const AVPacket* packet, AVFrame* frame, uint8_t* outBuffer,
                                int outSamples);  // 处理单个音频包
        void cleanup();                           // 资源清理
        void notifyError(const std::string& msg); // 线程安全的错误通知

        // --- C++17: 自定义删除器，用于智能指针管理 FFmpeg/ALSA 资源 ---
        template <typename T> struct FFmpegDeleter
        {
            void operator()(T* ptr) const noexcept
            {
                if constexpr (std::is_same_v<T, AVFormatContext>)
                {
                    if (ptr)
                    {
                        avformat_close_input(&ptr);
                    }
                }
                else if constexpr (std::is_same_v<T, AVCodecContext>)
                {
                    if (ptr)
                    {
                        avcodec_free_context(&ptr);
                    }
                }
                else if constexpr (std::is_same_v<T, SwrContext>)
                {
                    if (ptr)
                    {
                        swr_free(&ptr);
                    }
                }
                else if constexpr (std::is_same_v<T, snd_pcm_t>)
                {
                    if (ptr)
                    {
                        snd_pcm_drain(ptr);
                        snd_pcm_close(ptr);
                    }
                }
            }
        };

        template <typename T> using FFmpegPtr = std::unique_ptr<T, FFmpegDeleter<T>>;

        // --- 成员变量 ---
        // 状态控制 (原子操作，无需锁)
        std::atomic<bool> m_isPlaying{false};
        std::atomic<bool> m_isPaused{false};
        std::atomic<bool> m_stopRequested{false};

        // 音量
        std::atomic<float> m_volume{1.0F};

        // CPU亲和性设置
        int m_cpuAffinity{-1}; // -1表示使用默认CPU

        // 解码线程
        std::thread m_decodeThread;

        // 回调保护 (涉及跨线程调用，需要锁)
        std::mutex m_callbackMutex;
        PlaybackCallback m_onComplete;
        ErrorCallback m_onError;

        // --- FFmpeg 相关资源 (使用智能指针管理) ---
        FFmpegPtr<AVFormatContext> m_formatCtx{nullptr};
        FFmpegPtr<AVCodecContext> m_codecCtx{nullptr};
        FFmpegPtr<SwrContext> m_swrCtx{nullptr};
        int m_audioStreamIndex{-1};

        // --- ALSA 相关资源 ---
        FFmpegPtr<snd_pcm_t> m_pcmHandle{nullptr};

        // 播放进度
        std::atomic<double> m_currentPts{0.0};
    };
} // namespace hal