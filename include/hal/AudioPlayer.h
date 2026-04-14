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
        /**
         * @enum CommandType
         * @brief 控制命令类型枚举
         *
         * 定义所有可发送给控制线程的命令类型，用于串行化处理播放操作。
         */
        enum class CommandType : std::uint8_t
        {
            Play,    ///< 播放命令：开始播放指定音频文件
            Stop,    ///< 停止命令：停止当前播放并释放资源
            Pause,   ///< 暂停命令：暂停当前播放
            Resume,  ///< 恢复命令：从暂停状态恢复播放
            Seek,    ///< 跳转命令：跳转到指定播放位置
            Shutdown ///< 关闭命令：退出控制线程
        };

        /**
         * @struct SyncToken
         * @brief 同步令牌，用于命令的同步等待
         *
         * 当需要等待命令执行结果时（如 play/seek），使用此结构体进行线程同步。
         * 通过条件变量实现阻塞等待，直到命令执行完成。
         */
        struct SyncToken
        {
            std::mutex mutex;           ///< 保护同步状态的互斥锁
            std::condition_variable cv; ///< 用于等待命令完成的条件变量
            bool done{false};           ///< 命令是否已完成
            bool result{false};         ///< 命令执行结果（true=成功，false=失败）
        };

        /**
         * @struct Command
         * @brief 控制命令结构体
         *
         * 封装一个完整的控制命令，包含命令类型和相关参数。
         * 通过命令队列传递给控制线程执行。
         */
        struct Command
        {
            CommandType type{CommandType::Pause}; ///< 命令类型
            std::string filePath;                 ///< 音频文件路径（仅 Play 命令使用）
            double seekPositionSeconds{0.0};      ///< 跳转位置（仅 Seek 命令使用）
            std::shared_ptr<SyncToken> sync;      ///< 同步令牌（需要同步的命令使用）
        };

        // --- 内部私有方法 ---

        /**
         * @brief 控制线程主循环
         *
         * 在独立线程中运行，持续从命令队列中取出命令并执行。
         * 使用条件变量等待新命令，收到 Shutdown 命令后退出循环。
         */
        void controlLoop();

        /**
         * @brief 处理单个控制命令
         * @param command 要处理的命令
         *
         * 根据命令类型分发到对应的处理函数（handlePlayCommand、handleStopCommand 等）。
         * 如果命令带有同步令牌，处理完成后会通知等待线程。
         */
        void processCommand(const Command& command);

        /**
         * @brief 将命令加入队列（非阻塞）
         * @param command 要加入的命令（移动语义）
         *
         * 线程安全地将命令添加到命令队列末尾，并唤醒控制线程。
         * 不等待命令执行完成，适用于异步命令（如 pause/resume/stop）。
         */
        void enqueueCommand(Command&& command);

        /**
         * @brief 将命令加入队列并等待执行完成
         * @param command 要加入的命令（移动语义）
         * @param result 输出参数，接收命令执行结果
         * @return true 命令成功执行，false 超时或执行失败
         *
         * 线程安全地将命令添加到队列，并阻塞等待控制线程执行完成。
         * 适用于需要同步返回值的命令（如 play/seek）。
         * 如果等待超时（kSyncCommandTimeout），返回 false。
         */
        bool enqueueAndWait(Command&& command, bool& result);

        /**
         * @brief 设置同步令牌的结果并通知等待线程
         * @param sync 同步令牌指针
         * @param result 命令执行结果
         *
         * 在控制线程中调用，设置同步令牌的状态并唤醒等待的线程。
         * 静态函数，避免捕获 this 指针。
         */
        static void signalSyncResult(const std::shared_ptr<SyncToken>& sync, bool result);

        /**
         * @brief 检查当前是否在控制线程中执行
         * @return true 当前线程是控制线程，false 其他线程
         *
         * 用于调试和断言，确保某些方法只在控制线程中调用。
         */
        [[nodiscard]] bool isOnControlThread() const;

        /**
         * @brief 处理播放命令
         * @param filePath 音频文件路径
         * @return true 成功打开并开始播放，false 失败
         *
         * 打开音频文件，初始化解码器和音频输出，启动解码循环。
         * 如果正在播放其他文件，会先停止当前播放。
         */
        bool handlePlayCommand(const std::string& filePath);

        /**
         * @brief 处理停止命令
         *
         * 停止解码循环，关闭音频输出，释放播放资源。
         * 重置播放状态为空闲。
         */
        void handleStopCommand();

        /**
         * @brief 处理暂停命令
         *
         * 暂停音频输出，保持解码器状态。
         * 如果未处于播放状态或已暂停，此操作无效。
         */
        void handlePauseCommand();

        /**
         * @brief 处理恢复播放命令
         *
         * 从暂停状态恢复音频输出。
         * 如果未处于暂停状态，此操作无效。
         */
        void handleResumeCommand();

        /**
         * @brief 处理跳转命令
         * @param positionSeconds 目标播放位置（秒）
         * @return true 跳转成功，false 失败
         *
         * 跳转到指定播放位置，刷新解码器缓冲区。
         * 更新当前播放进度，可能需要重新填充 ALSA 缓冲区。
         */
        bool handleSeekCommand(double positionSeconds);

        /**
         * @brief 执行一步解码和播放
         *
         * 从解码器获取一帧音频数据，发送到音频输出设备。
         * 在控制线程中循环调用，直到播放完成或收到停止命令。
         * 如果解码器缓冲区为空，会自动填充更多数据。
         */
        void decodeOneStep();

        /**
         * @brief 重置播放资源
         *
         * 释放解码器和音频输出对象，重置播放状态变量。
         * 在停止播放或开始新播放前调用，确保资源正确释放。
         */
        void resetPlaybackResources();

        /**
         * @brief 回调分发线程主循环
         *
         * 在独立线程中运行，持续从回调队列中取出任务并执行。
         * 将外部回调从控制线程分离，避免回调阻塞影响播放性能。
         * 收到退出标志后清空队列并退出。
         */
        void callbackLoop();

        /**
         * @brief 将回调任务加入队列
         * @param task 要执行的回调函数
         *
         * 线程安全地将回调任务添加到回调队列，并唤醒回调线程。
         * 回调会在回调分发线程中异步执行，不会阻塞调用者。
         */
        void postCallbackTask(std::function<void()> task);

        /**
         * @brief 通知播放完成
         *
         * 当音频文件播放完毕时调用，通过回调队列触发 onComplete 回调。
         * 自动重置播放状态，准备下一次播放。
         */
        void notifyPlaybackComplete();

        /**
         * @brief 线程安全地触发错误回调
         * @param msg 错误信息
         *
         * 通过回调队列异步触发 onError 回调，避免在控制线程中直接调用。
         * 自动检查回调是否已设置，未设置时不执行任何操作。
         */
        void notifyError(const std::string& msg);

        // --- 成员变量 ---

        // 播放状态标志（原子操作，跨线程访问）
        std::atomic<bool> m_isPlaying{false}; ///< 是否正在播放（包括暂停状态）
        std::atomic<bool> m_isPaused{false};  ///< 是否处于暂停状态

        // 音量控制（原子操作，可在任意线程设置）
        std::atomic<float> m_volume{1.0F}; ///< 当前音量值，范围 0.0（静音）~ 1.0（最大音量）

        // CPU亲和性配置
        int m_cpuAffinity{-1}; ///< 解码线程绑定的CPU核心ID，-1表示使用默认策略

        // 控制线程及命令队列（单消费者模型）
        std::thread m_controlThread;         ///< 控制线程对象，负责串行处理所有播放命令
        std::thread::id m_controlThreadId;   ///< 控制线程ID，用于判断当前是否在控制线程中
        std::mutex m_commandMutex;           ///< 保护命令队列的互斥锁
        std::condition_variable m_commandCv; ///< 通知控制线程有新命令的条件变量
        std::deque<Command> m_commandQueue;  ///< 待执行的命令队列（FIFO）
        bool m_exitRequested{false};         ///< 控制线程退出标志，设为true后线程将退出

        // 回调分发线程及回调队列（独立于控制线程）
        std::thread m_callbackThread;                      ///< 回调分发线程对象，负责异步执行外部回调
        std::mutex m_callbackQueueMutex;                   ///< 保护回调队列的互斥锁
        std::condition_variable m_callbackQueueCv;         ///< 通知回调线程有新任务的条件变量
        std::deque<std::function<void()>> m_callbackQueue; ///< 待执行的回调任务队列
        bool m_callbackExitRequested{false};               ///< 回调线程退出标志

        // 回调函数保护（跨线程访问需要锁）
        std::mutex m_callbackMutex;    ///< 保护回调函数指针的互斥锁
        PlaybackCallback m_onComplete; ///< 播放完成回调函数
        ErrorCallback m_onError;       ///< 错误发生时的回调函数

        // 音频解码和输出设备
        std::unique_ptr<AudioDecoder> m_decoder; ///< 音频解码器，负责解码音频文件
        std::unique_ptr<AudioOutput> m_output;   ///< 音频输出设备，负责通过ALSA播放音频

        // 播放进度信息（原子操作，跨线程访问）
        std::atomic<double> m_currentPts{0.0};      ///< 当前播放位置（PTS，以秒为单位）
        std::atomic<double> m_durationSeconds{0.0}; ///< 音频总时长（秒）
        std::atomic<bool> m_hasDuration{false};     ///< 是否成功获取到音频总时长

        // 常量定义
        static constexpr std::chrono::milliseconds kSyncCommandTimeout{5000}; ///< 同步命令超时时间（5秒）
    };
} // namespace hal