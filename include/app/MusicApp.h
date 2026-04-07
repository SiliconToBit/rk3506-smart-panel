#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <mutex>
#include <random>

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
         * @brief 设置进度变化回调
         * @param callback 回调函数
         */
        void setOnProgressChanged(ProgressCallback callback);

        /**
         * @brief 设置歌曲切换回调
         * @param callback 回调函数
         */
        void setOnSongChanged(SongChangedCallback callback);

        /**
         * @brief 设置播放状态变化回调
         * @param callback 回调函数
         */
        void setOnStateChanged(StateChangedCallback callback);

        /**
         * @brief 设置错误回调
         * @param callback 回调函数
         */
        void setOnError(ErrorCallback callback);

    private:
        MusicApp();
        ~MusicApp();

        // 禁止拷贝
        MusicApp(const MusicApp&) = delete;
        MusicApp& operator=(const MusicApp&) = delete;

        // 内部方法
        void setupAudioCallbacks();
        void notifyProgressChanged();
        void notifySongChanged();
        void notifyStateChanged(bool playing);
        void notifyError(const std::string& msg);
        std::string extractSongName(const std::string& filePath) const;
        bool isAudioFile(const std::string& filePath) const;
        int getNextIndex() const;
        int getPrevIndex() const;

        // 成员变量
        std::unique_ptr<hal::AudioPlayer> m_audioPlayer;
        std::vector<std::string> m_playlist;
        int m_currentIndex = -1;
        PlayMode m_playMode = PlayMode::Sequence;
        float m_volume = 1.0f;

        // 随机播放用（mutable 允许在 const 方法中修改）
        mutable std::mt19937 m_rng;

        // 回调
        std::mutex m_callbackMutex;
        ProgressCallback m_onProgressChanged;
        SongChangedCallback m_onSongChanged;
        StateChangedCallback m_onStateChanged;
        ErrorCallback m_onError;
    };

} // namespace app
