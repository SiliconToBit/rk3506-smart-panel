#pragma once

#include <atomic>
#include <condition_variable>
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
    /**
     * @class AudioPlayer
     * @brief 基于 FFmpeg + ALSA 的音频播放器
     *
     * 支持常见音频格式的解码播放，提供播放控制、音量调节、Seek 等功能。
     * 使用独立解码线程避免阻塞调用方，通过智能指针管理 FFmpeg/ALSA 资源。
     *
     * @note 该类不可拷贝，同一实例一次只能播放一个文件
     */
    class AudioPlayer
    {
    public:
        // 状态回调类型定义
        using PlaybackCallback = std::function<void()>;
        using ErrorCallback = std::function<void(const std::string& errorMsg)>;

        /**
         * @brief 构造 AudioPlayer 实例
         */
        AudioPlayer();

        /**
         * @brief 析构函数，停止播放并释放所有资源
         */
        ~AudioPlayer();

        // 禁止拷贝和赋值（C++11 起）
        AudioPlayer(const AudioPlayer&) = delete;
        AudioPlayer& operator=(const AudioPlayer&) = delete;

        // --- 公开接口 ---

        /**
         * @brief 开始播放指定音频文件
         * @param filePath 音频文件路径（支持 MP3/AAC/FLAC/WAV 等 FFmpeg 支持的格式）
         * @return true 成功启动播放，false 打开文件或初始化解码器失败
         * @note 如果正在播放，会先停止当前播放再开始新的播放
         * @note setCpuAffinity() 必须在 play() 之前调用才有效
         */
        bool play(const std::string& filePath);

        /**
         * @brief 停止播放并释放资源
         * @note 如果未处于播放状态，此操作无效
         * @note 调用后需要重新调用 play() 才能播放
         */
        void stop();

        /**
         * @brief 暂停播放
         * @note 如果未播放或已暂停，此操作无效
         */
        void pause();

        /**
         * @brief 从暂停状态恢复播放
         * @note 如果未处于暂停状态，此操作无效
         */
        void resume();

        /**
         * @brief 设置播放音量
         * @param volume 音量值，范围 0.0（静音）~ 1.0（最大音量）
         * @note 超出范围的值会被自动截断到有效区间
         */
        void setVolume(float volume);

        /**
         * @brief 获取当前播放进度
         * @return 当前播放位置（秒），如果未播放或无法获取则返回 std::nullopt
         */
        [[nodiscard]] std::optional<double> getCurrentPosition() const;

        /**
         * @brief 获取音频总时长
         * @return 音频总时长（秒），如果无法获取则返回 std::nullopt
         * @note 某些流媒体或损坏文件可能无法获取时长
         */
        [[nodiscard]] std::optional<double> getDuration() const;

        /**
         * @brief 跳转到指定播放位置
         * @param positionSeconds 目标位置（秒），从文件开头计算
         * @return true 跳转成功，false 未播放或跳转失败
         * @note 跳转后会刷新解码器缓冲区，但 ALSA 缓冲区中可能残留旧数据
         */
        bool seek(double positionSeconds);

        /**
         * @brief 检查是否处于暂停状态
         * @return true 正在播放且已暂停，false 其他状态
         */
        [[nodiscard]] bool isPaused() const;

        /**
         * @brief 检查是否正在播放
         * @return true 正在播放（包括暂停状态），false 未播放
         */
        [[nodiscard]] bool isPlaying() const;

        /**
         * @brief 设置播放完成回调
         * @param callback 播放完成时调用的函数（在解码线程中执行）
         * @note 回调函数应避免耗时操作，以免阻塞解码线程
         */
        void setOnPlaybackComplete(PlaybackCallback callback);

        /**
         * @brief 设置错误回调
         * @param callback 发生错误时调用的函数，参数为错误信息（在解码线程中执行）
         * @note 回调函数应避免耗时操作，以免阻塞解码线程
         */
        void setOnError(ErrorCallback callback);

        /**
         * @brief 设置解码线程的 CPU 亲和性
         * @param cpuId 目标 CPU 核心 ID（从 0 开始）
         * @note 必须在 play() 之前调用，播放过程中修改无效
         * @note 传入 -1 表示使用默认 CPU 选择策略
         */
        void setCpuAffinity(int cpuId);

    private:
        static constexpr int kOutputChannels = 2;
        static constexpr int kOutputSampleRate = 44100;

        // --- 内部私有方法 ---

        /**
         * @brief 打开音频文件并初始化 FFmpeg 解码器、重采样器、ALSA 输出
         * @param filePath 音频文件路径
         * @return true 初始化成功，false 失败
         */
        bool openFile(const std::string& filePath);

        /**
         * @brief 解码线程主循环，负责读取、解码、重采样、播放
         */
        void decodeLoop();

        /**
         * @brief 将解码后的音频帧重采样为目标格式
         * @param frame 解码后的音频帧
         * @param outBuffer 输出缓冲区指针（由调用方管理）
         * @param outSamples 最大输出采样数
         * @return 实际输出的采样数，<=0 表示失败
         */
        int convertFrameSamples(AVFrame* frame, uint8_t*& outBuffer, int outSamples) const;

        /**
         * @brief 对 PCM 数据应用软件音量控制
         * @param outBuffer PCM 数据缓冲区（S16 格式）
         * @param sampleCount 采样数（每个声道）
         */
        void applySoftwareVolume(uint8_t* outBuffer, int sampleCount) const;

        /**
         * @brief 将 PCM 数据写入 ALSA 设备
         * @param outBuffer PCM 数据缓冲区
         * @param sampleCount 采样数
         * @note 写入失败时会自动尝试恢复 ALSA 设备
         */
        void writePcmSamples(uint8_t* outBuffer, int sampleCount);

        /**
         * @brief 根据音频帧时间戳更新当前播放进度
         * @param frame 当前播放的音频帧
         */
        void updatePlaybackPosition(const AVFrame* frame);

        /**
         * @brief 处理单个音频包：解码、重采样、写入 ALSA
         * @param packet 音频数据包
         * @param frame 用于接收解码帧的缓冲区
         * @param outBuffer 重采样输出缓冲区
         * @param outSamples 最大输出采样数
         */
        void processAudioPacket(const AVPacket* packet, AVFrame* frame, uint8_t* outBuffer, int outSamples);

        /**
         * @brief 清理播放资源（重置状态，智能指针自动释放 FFmpeg/ALSA 资源）
         */
        void cleanup();

        /**
         * @brief 线程安全地触发错误回调
         * @param msg 错误信息
         */
        void notifyError(const std::string& msg);

        /**
         * @brief 等待暂停状态解除或停止请求
         * @return true 应继续播放，false 收到停止请求
         */
        bool waitForResumeOrStop();

        /**
         * @brief 唤醒等待中的解码线程（用于暂停/恢复控制）
         */
        void notifyPauseCondition();

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

        // 暂停/恢复同步机制（替代轮询，提高实时性）
        std::mutex m_pauseMutex;
        std::condition_variable m_pauseCondition;

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