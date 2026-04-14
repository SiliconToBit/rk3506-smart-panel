#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <vector>

#include "app/LyricParser.h"

extern "C"
{
#include "lvgl.h"
}

namespace hal
{
    class AudioPlayer;
}

namespace app
{

    /**
     * @brief 音乐播放应用 - 管理播放列表和播放状态
     *
     * 职责：
     * - 播放列表管理（加载目录、切换歌曲）
     * - 播放状态管理（播放/暂停/停止/上下首）
     * - UI 状态回调（进度更新、歌曲切换通知）
     *
     * 使用示例：
     * @code
     * // 在 eez_actions.cpp 中调用
     * void action_toggle_play_pause(lv_event_t* e) {
     *     auto& app = app::MusicApp::getInstance();
     *     if (app.isPlaying()) {
     *         app.pause();
     *     } else {
     *         app.resume();
     *     }
     * }
     *
     * void action_play_next(lv_event_t* e) {
     *     app::MusicApp::getInstance().next();
     * }
     *
     * void action_music_volume_changed(lv_event_t* e) {
     *     lv_obj_t* slider = lv_event_get_target(e);
     *     int value = lv_slider_get_value(slider);
     *     app::MusicApp::getInstance().setVolume(value / 100.0f);
     * }
     *
     * void action_music_progress_changed(lv_event_t* e) {
     *     lv_obj_t* slider = lv_event_get_target(e);
     *     int value = lv_slider_get_value(slider);  // 0-100
     *     auto& app = app::MusicApp::getInstance();
     *     double duration = app.getDuration();
     *     if (duration > 0) {
     *         app.seek(duration * value / 100.0);
     *     }
     * }
     *
     * // 设置回调更新 UI
     * app::MusicApp::getInstance().setOnProgressChanged([](const auto& info) {
     *     // 更新进度条和标签
     *     lv_slider_set_value(music_progress_slider, info.progressPercent * 100, LV_ANIM_OFF);
     *     lv_label_set_text_fmt(music_curr_time_label, "%d:%02d",
     *         (int)info.currentPosition / 60, (int)info.currentPosition % 60);
     * });
     *
     * app::MusicApp::getInstance().setOnSongChanged([](int index, const std::string& name) {
     *     lv_label_set_text(music_title_label, name.c_str());
     * });
     * @endcode
     */
    class MusicApp
    {
    public:
        // ========== 单例访问 ==========
        /**
         * @brief 获取单例实例
         * @return MusicApp 单例引用
         */
        static MusicApp& getInstance();

        // ========== 播放列表管理 ==========

        /**
         * @brief 加载音乐目录，自动扫描支持的音频文件
         * @param directoryPath 目录路径，如 "/mnt/music"
         * @return 加载的文件数量
         *
         * 支持的格式：.mp3, .wav, .flac, .aac, .ogg, .m4a
         */
        int loadDirectory(const std::string& directoryPath);

        /**
         * @brief 清空播放列表
         */
        void clearPlaylist();

        /**
         * @brief 获取播放列表
         * @return 文件路径列表
         */
        [[nodiscard]] std::vector<std::string> getPlaylist() const;

        /**
         * @brief 获取播放列表大小
         * @return 歌曲数量
         */
        [[nodiscard]] int getPlaylistSize() const;

        /**
         * @brief 获取当前播放索引
         * @return 当前索引，-1 表示无歌曲播放
         */
        [[nodiscard]] int getCurrentIndex() const;

        /**
         * @brief 获取当前歌曲名称（不含路径和扩展名）
         * @return 歌曲名称
         */
        [[nodiscard]] std::string getCurrentSongName() const;

        /**
         * @brief 获取指定索引的歌曲名称
         * @param index 播放列表索引
         * @return 歌曲名称
         */
        [[nodiscard]] std::string getSongName(int index) const;

        // ========== 播放控制 ==========

        /**
         * @brief 播放指定索引的歌曲
         * @param index 播放列表索引
         * @return 是否成功开始播放
         */
        bool play(int index);

        /**
         * @brief 播放指定路径的文件（自动添加到列表）
         * @param filePath 文件完整路径
         * @return 是否成功开始播放
         */
        bool play(const std::string& filePath);

        /**
         * @brief 暂停播放
         */
        void pause();

        /**
         * @brief 恢复播放
         */
        void resume();

        /**
         * @brief 停止播放
         */
        void stop();

        /**
         * @brief 播放下一首
         * @return 是否成功（false 表示已到末尾或列表为空）
         */
        bool next();

        /**
         * @brief 播放上一首
         * @return 是否成功（false 表示已在开头或列表为空）
         */
        bool prev();

        /**
         * @brief 跳转到指定进度位置
         * @param positionSeconds 目标位置（秒）
         */
        void seek(double positionSeconds);

        // ========== 音量控制 ==========

        /**
         * @brief 设置音量
         * @param volume 音量值 [0.0, 1.0]
         */
        void setVolume(float volume);

        /**
         * @brief 获取当前音量
         * @return 音量值 [0.0, 1.0]
         */
        [[nodiscard]] float getVolume() const;

        // ========== 状态查询 ==========

        /**
         * @brief 是否正在播放
         * @return true 表示正在播放
         */
        [[nodiscard]] bool isPlaying() const;

        /**
         * @brief 是否暂停中
         * @return true 表示暂停状态
         */
        [[nodiscard]] bool isPaused() const;

        /**
         * @brief 获取当前播放进度（秒）
         * @return 当前位置，0 表示无播放
         */
        [[nodiscard]] double getCurrentPosition() const;

        /**
         * @brief 获取当前歌曲总时长（秒）
         * @return 总时长，0 表示无播放或无法获取
         */
        [[nodiscard]] double getDuration() const;

        /**
         * @brief 获取播放进度百分比 [0.0, 1.0]
         * @return 进度百分比
         */
        [[nodiscard]] float getProgressPercent() const;

        // ========== 播放模式 ==========

        /**
         * @brief 播放模式枚举
         */
        enum class PlayMode
        {
            Sequence,   ///< 顺序播放，播完停止
            Loop,       ///< 列表循环，播完从头开始
            SingleLoop, ///< 单曲循环，重复当前歌曲
            Shuffle     ///< 随机播放
        };

        /**
         * @brief 设置播放模式
         * @param mode 播放模式
         */
        void setPlayMode(PlayMode mode);

        /**
         * @brief 获取当前播放模式
         * @return 播放模式
         */
        [[nodiscard]] PlayMode getPlayMode() const;

        // ========== UI 回调 ==========

        /**
         * @brief 播放进度信息结构
         */
        struct ProgressInfo
        {
            double currentPosition; ///< 当前位置（秒）
            double duration;        ///< 总时长（秒）
            float progressPercent;  ///< 进度百分比 [0.0, 1.0]
        };

        /**
         * @brief 进度变化回调类型
         */
        using ProgressCallback = std::function<void(const ProgressInfo&)>;

        /**
         * @brief 歌曲切换回调类型
         * @param index 新歌曲索引
         * @param songName 歌曲名称
         */
        using SongChangedCallback = std::function<void(int index, const std::string& songName)>;

        /**
         * @brief 播放状态变化回调类型
         * @param isPlaying true 表示开始播放，false 表示停止
         */
        using StateChangedCallback = std::function<void(bool isPlaying)>;

        /**
         * @brief 错误回调类型
         * @param errorMsg 错误消息
         */
        using ErrorCallback = std::function<void(const std::string& errorMsg)>;

        /**
         * @brief 歌词变化回调类型
         * @param lyric 当前歌词行文本
         */
        using LyricChangedCallback = std::function<void(const std::string& lyric)>;

        /**
         * @brief 设置进度变化回调
         * @param callback 回调函数
         * @note 回调在 LVGL 线程中执行，可安全操作 LVGL 控件
         */
        void setOnProgressChanged(ProgressCallback callback);

        /**
         * @brief 设置歌曲切换回调
         * @param callback 回调函数
         * @note 回调在 LVGL 线程中执行，可安全操作 LVGL 控件
         */
        void setOnSongChanged(SongChangedCallback callback);

        /**
         * @brief 设置播放状态变化回调
         * @param callback 回调函数
         * @note 回调在 LVGL 线程中执行，可安全操作 LVGL 控件
         */
        void setOnStateChanged(StateChangedCallback callback);

        /**
         * @brief 设置错误回调
         * @param callback 回调函数
         * @note 回调在 LVGL 线程中执行，可安全操作 LVGL 控件
         */
        void setOnError(ErrorCallback callback);

        /**
         * @brief 设置歌词变化回调
         * @param callback 回调函数
         * @note 回调在 LVGL 线程中执行，可安全操作 LVGL 控件
         */
        void setOnLyricChanged(LyricChangedCallback callback);

        /**
         * @brief 启动进度更新定时器（需在 LVGL 初始化后调用）
         * @param periodMs 更新周期（毫秒），默认 500ms
         * @note 此方法必须在 LVGL 线程中调用
         */
        void startProgressTimer(uint32_t periodMs = 500);

        /**
         * @brief 停止进度更新定时器
         */
        void stopProgressTimer();

    private:
        /**
         * @brief 私有构造函数（单例模式）
         *
         * 初始化音频播放器、随机数生成器等成员变量。
         * 只能通过 getInstance() 获取实例。
         */
        MusicApp();

        /**
         * @brief 私有析构函数（单例模式）
         *
         * 停止进度定时器，释放音频播放器资源。
         */
        ~MusicApp();

        /**
         * @brief 禁止拷贝构造（单例模式）
         */
        MusicApp(const MusicApp&) = delete;

        /**
         * @brief 禁止赋值操作（单例模式）
         */
        MusicApp& operator=(const MusicApp&) = delete;

        // --- 内部方法 ---

        /**
         * @brief 配置音频播放器的回调函数
         *
         * 将 AudioPlayer 的底层回调（播放完成、错误）绑定到 MusicApp 的内部处理函数。
         * 在构造函数中调用一次，确保音频事件能正确传递到 UI 层。
         */
        void setupAudioCallbacks();

        /**
         * @brief 通知进度变化
         *
         * 获取当前播放进度，构造 ProgressInfo 结构体，调用进度回调。
         * 由进度定时器周期性调用（默认 500ms）。
         * 如果未设置回调或无歌曲播放，不执行任何操作。
         */
        void notifyProgressChanged();

        /**
         * @brief 通知歌曲切换
         *
         * 当播放新歌曲时调用，通过 LVGL 异步机制触发歌曲切换回调。
         * 使用 pending 标志防止重复调度，确保回调只执行一次。
         * 同时加载对应的歌词文件。
         */
        void notifySongChanged();

        /**
         * @brief 通知播放状态变化
         * @param playing true 表示开始播放，false 表示停止
         *
         * 当播放状态改变时调用（播放/暂停/停止），通过 LVGL 异步机制触发状态回调。
         * 使用 pending 标志防止重复调度。
         */
        void notifyStateChanged(bool playing);

        /**
         * @brief 通知错误发生
         * @param msg 错误信息
         *
         * 当音频播放器发生错误时调用，通过 LVGL 异步机制触发错误回调。
         * 使用 pending 标志防止重复调度。
         */
        void notifyError(const std::string& msg);

        /**
         * @brief 为当前歌曲加载歌词
         *
         * 根据当前音频文件路径查找对应的歌词文件（.lrc 格式）。
         * 查找策略：
         * 1. 同名 .lrc 文件（如 song.mp3 -> song.lrc）
         * 2. 同一目录下的歌词文件
         *
         * 加载成功后解析歌词时间戳，供播放时同步显示。
         * 如果找不到歌词文件或解析失败，不执行任何操作。
         */
        void loadLyricsForCurrentSong();

        /**
         * @brief 通知歌词变化
         * @param positionSeconds 当前播放位置（秒）
         *
         * 根据播放进度查找当前应显示的歌词行，触发歌词回调。
         * 避免重复触发相同歌词行（通过 m_lastLyricText 比较）。
         * 由进度定时器调用，与进度更新同步。
         */
        void notifyLyricChanged(double positionSeconds);

        /**
         * @brief 从文件路径提取歌曲名称
         * @param filePath 完整文件路径（如 "/mnt/music/song.mp3"）
         * @return 歌曲名称（不含路径和扩展名，如 "song"）
         *
         * 用于 UI 显示，去除路径前缀和文件扩展名。
         * 支持多种扩展名（.mp3, .wav, .flac, .aac, .ogg, .m4a）。
         */
        [[nodiscard]] std::string extractSongName(const std::string& filePath) const;

        /**
         * @brief 判断文件是否为支持的音频格式
         * @param filePath 文件路径
         * @return true 是支持的音频格式，false 不支持
         *
         * 检查文件扩展名是否在支持的格式列表中。
         * 支持的格式：.mp3, .wav, .flac, .aac, .ogg, .m4a
         * 比较时不区分大小写。
         */
        [[nodiscard]] bool isAudioFile(const std::string& filePath) const;

        /**
         * @brief 获取下一首歌曲的索引
         * @return 下一首的索引，-1 表示无下一首
         *
         * 根据当前播放模式计算下一首歌曲：
         * - Sequence：返回当前索引 + 1，如果是最后一首返回 -1
         * - Loop：返回当前索引 + 1，如果是最后一首返回 0
         * - SingleLoop：返回当前索引（重复播放）
         * - Shuffle：随机返回一个索引（可能与当前相同）
         *
         * @note Shuffle 模式使用 m_rng 生成随机数，线程安全
         */
        [[nodiscard]] int getNextIndex() const;

        /**
         * @brief 获取上一首歌曲的索引
         * @return 上一首的索引，-1 表示无上一首
         *
         * 根据当前播放模式计算上一首歌曲：
         * - Sequence：返回当前索引 - 1，如果是第一首返回 -1
         * - Loop：返回当前索引 - 1，如果是第一首返回最后一首
         * - SingleLoop：返回当前索引（重复播放）
         * - Shuffle：随机返回一个索引
         *
         * @note Shuffle 模式使用 m_rng 生成随机数，线程安全
         */
        [[nodiscard]] int getPrevIndex() const;

        // --- LVGL 异步回调 Thunk（静态函数，适配 lv_async_call）---

        /**
         * @brief 歌曲切换异步回调入口
         * @param userData 用户数据指针（由 LVGL 传递）
         *
         * 静态函数，作为 lv_async_call 的回调入口。
         * 从 userData 中恢复上下文，调用实际的歌曲切换通知。
         * 在 LVGL 线程中执行，可安全操作 LVGL 控件。
         */
        static void asyncSongChangedThunk(void* userData);

        /**
         * @brief 状态变化异步回调入口
         * @param userData 用户数据指针（由 LVGL 传递）
         *
         * 静态函数，作为 lv_async_call 的回调入口。
         * 从 userData 中恢复上下文，调用实际的状态变化通知。
         * 在 LVGL 线程中执行，可安全操作 LVGL 控件。
         */
        static void asyncStateChangedThunk(void* userData);

        /**
         * @brief 错误异步回调入口
         * @param userData 用户数据指针（由 LVGL 传递）
         *
         * 静态函数，作为 lv_async_call 的回调入口。
         * 从 userData 中恢复上下文，调用实际的错误通知。
         * 在 LVGL 线程中执行，可安全操作 LVGL 控件。
         */
        static void asyncErrorThunk(void* userData);

        /**
         * @brief 自动播放下一首异步回调入口
         * @param userData 用户数据指针（由 LVGL 传递）
         *
         * 静态函数，作为 lv_async_call 的回调入口。
         * 当一首歌播放完成时，通过此回调自动切换到下一首。
         * 在 LVGL 线程中执行，根据播放模式决定下一首歌曲。
         */
        static void asyncAutoNextThunk(void* userData);

        /**
         * @brief 进度定时器回调函数
         * @param timer LVGL 定时器指针
         *
         * 由 LVGL 定时器系统周期性调用（默认 500ms）。
         * 触发进度更新和歌词同步，调用 notifyProgressChanged() 和 notifyLyricChanged()。
         * 在 LVGL 线程中执行，可安全操作 LVGL 控件。
         */
        static void progressTimerCallback(lv_timer_t* timer);

        // --- 成员变量 ---

        // 音频播放器（底层播放引擎）
        std::unique_ptr<hal::AudioPlayer> m_audioPlayer; ///< 音频播放器实例，负责实际解码和播放

        // 播放列表相关
        std::vector<std::string> m_playlist;  ///< 播放列表，存储音频文件完整路径
        int m_currentIndex = -1;              ///< 当前播放的歌曲索引，-1 表示无歌曲
        PlayMode m_playMode = PlayMode::Loop; ///< 当前播放模式，默认为列表循环
        float m_volume = 1.0f;                ///< 当前音量值，范围 0.0（静音）~ 1.0（最大音量）

        // 随机播放相关（mutable 允许在 const 方法中修改）
        mutable std::mt19937 m_rng;    ///< Mersenne Twister 随机数生成器，用于 Shuffle 模式
        mutable std::mutex m_rngMutex; ///< 保护 m_rng 的互斥锁，确保多线程安全

        // 进度定时器
        lv_timer_t* m_progressTimer = nullptr; ///< LVGL 定时器指针，用于周期性更新进度和歌词

        // LVGL 异步调用 pending 标志（防止重复调度）
        std::atomic<bool> m_songChangedPending{false};  ///< 歌曲切换回调是否已调度待执行
        std::atomic<bool> m_stateChangedPending{false}; ///< 状态变化回调是否已调度待执行
        std::atomic<bool> m_errorPending{false};        ///< 错误回调是否已调度待执行
        std::atomic<bool> m_autoNextPending{false};     ///< 自动播放下一首是否已调度待执行

        // 待传递的歌曲切换数据（在异步回调执行前有效）
        struct PendingSongData
        {
            int index = -1;       ///< 待切换的歌曲索引
            std::string songName; ///< 待切换的歌曲名称
        };
        PendingSongData m_pendingSongData; ///< 待传递的歌曲数据
        std::mutex m_pendingSongMutex;     ///< 保护 m_pendingSongData 的互斥锁

        // 待传递的状态变化数据（在异步回调执行前有效）
        struct PendingStateData
        {
            bool isPlaying = false; ///< 待传递的播放状态
        };
        PendingStateData m_pendingStateData;          ///< 待传递的状态数据
        std::mutex m_pendingStateMutex;               ///< 保护 m_pendingStateData 的互斥锁
        std::atomic<bool> m_lastNotifiedState{false}; ///< 上次通知的播放状态，用于避免重复通知

        // 待传递的错误数据（在异步回调执行前有效）
        struct PendingErrorData
        {
            std::string errorMsg; ///< 待传递的错误信息
        };
        PendingErrorData m_pendingErrorData; ///< 待传递的错误数据
        std::mutex m_pendingErrorMutex;      ///< 保护 m_pendingErrorData 的互斥锁

        // 回调函数（跨线程访问需要锁保护）
        std::mutex m_callbackMutex;            ///< 保护所有回调函数的互斥锁
        ProgressCallback m_onProgressChanged;  ///< 进度变化回调函数
        SongChangedCallback m_onSongChanged;   ///< 歌曲切换回调函数
        StateChangedCallback m_onStateChanged; ///< 播放状态变化回调函数
        ErrorCallback m_onError;               ///< 错误回调函数
        LyricChangedCallback m_onLyricChanged; ///< 歌词变化回调函数

        // 歌词相关
        std::mutex m_lyricMutex;     ///< 保护歌词解析器的互斥锁
        LyricParser m_lyricParser;   ///< 歌词解析器，解析 .lrc 文件
        std::string m_lastLyricText; ///< 上次显示的歌词文本，用于避免重复触发
    };

} // namespace app
