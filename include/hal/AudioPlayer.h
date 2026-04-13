#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

namespace hal
{
    class AudioDecoder;
    class AudioOutput;
} // namespace hal

namespace hal
{
    /**
     * @class AudioPlayer
     * @brief 基于 FFmpeg + ALSA 的音频播放器
     *
     * 支持常见音频格式的解码播放，提供播放控制、音量调节、Seek 等功能。
     * 使用单控制线程串行处理命令，使用独立回调分发线程避免外部回调阻塞播放控制。
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
         * @param callback 播放完成时调用的函数（在回调分发线程中执行）
         * @note 回调函数应避免长时间阻塞，建议将业务切回 UI/主线程
         */
        void setOnPlaybackComplete(PlaybackCallback callback);

        /**
         * @brief 设置错误回调
         * @param callback 发生错误时调用的函数，参数为错误信息（在回调分发线程中执行）
         * @note 回调函数应避免长时间阻塞，建议将业务切回 UI/主线程
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
        enum class CommandType : std::uint8_t
        {
            Play,
            Stop,
            Pause,
            Resume,
            Seek,
            Shutdown
        };

        struct SyncToken
        {
            std::mutex mutex;
            std::condition_variable cv;
            bool done{false};
            bool result{false};
        };

        struct Command
        {
            CommandType type{CommandType::Pause};
            std::string filePath;
            double seekPositionSeconds{0.0};
            std::shared_ptr<SyncToken> sync;
        };

        // --- 内部私有方法 ---
        void controlLoop();
        void processCommand(const Command& command);
        void enqueueCommand(Command&& command);
        bool enqueueAndWait(Command&& command, bool& result);
        static void signalSyncResult(const std::shared_ptr<SyncToken>& sync, bool result);
        [[nodiscard]] bool isOnControlThread() const;

        bool handlePlayCommand(const std::string& filePath);
        void handleStopCommand();
        void handlePauseCommand();
        void handleResumeCommand();
        bool handleSeekCommand(double positionSeconds);
        void decodeOneStep();
        void resetPlaybackResources();
        void callbackLoop();
        void postCallbackTask(std::function<void()> task);
        void notifyPlaybackComplete();

        /**
         * @brief 线程安全地触发错误回调
         * @param msg 错误信息
         */
        void notifyError(const std::string& msg);

        // --- 成员变量 ---
        // 状态控制
        std::atomic<bool> m_isPlaying{false};
        std::atomic<bool> m_isPaused{false};

        // 音量
        std::atomic<float> m_volume{1.0F};

        // CPU亲和性设置
        int m_cpuAffinity{-1}; // -1表示使用默认CPU

        // 单控制线程 + 命令队列
        std::thread m_controlThread;
        std::thread::id m_controlThreadId;
        std::mutex m_commandMutex;
        std::condition_variable m_commandCv;
        std::deque<Command> m_commandQueue;
        bool m_exitRequested{false};

        // 回调分发线程，防止控制线程被外部回调阻塞
        std::thread m_callbackThread;
        std::mutex m_callbackQueueMutex;
        std::condition_variable m_callbackQueueCv;
        std::deque<std::function<void()>> m_callbackQueue;
        bool m_callbackExitRequested{false};

        // 回调保护 (涉及跨线程调用，需要锁)
        std::mutex m_callbackMutex;
        PlaybackCallback m_onComplete;
        ErrorCallback m_onError;

        std::unique_ptr<AudioDecoder> m_decoder;
        std::unique_ptr<AudioOutput> m_output;

        // 播放进度
        std::atomic<double> m_currentPts{0.0};
        std::atomic<double> m_durationSeconds{0.0};
        std::atomic<bool> m_hasDuration{false};

        static constexpr std::chrono::milliseconds kSyncCommandTimeout{5000};
    };
} // namespace hal