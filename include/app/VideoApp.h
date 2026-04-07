#pragma once

#include <string>
#include <functional>
#include <memory>
#include <mutex>

namespace hal
{
    class VideoPlayer;
}

namespace app
{

    /**
     * @brief 视频播放应用 - 管理视频播放和显示
     *
     * 职责：
     * - 视频播放控制
     * - LVGL 显示区域管理
     * - 播放状态管理
     * - UI 状态回调
     *
     * 使用示例：
     * @code
     * // 在 eez_actions.cpp 中调用
     * void action_init_video_screen(lv_event_t* e) {
     *     app::VideoApp::getInstance().setDisplayArea(0, 0, 800, 480);
     * }
     *
     * void action_video_play(lv_event_t* e) {
     *     app::VideoApp::getInstance().play("/mnt/video/sample.mp4");
     * }
     *
     * void action_video_toggle_pause(lv_event_t* e) {
     *     auto& app = app::VideoApp::getInstance();
     *     if (app.isPlaying()) {
     *         app.pause();
     *     } else {
     *         app.resume();
     *     }
     * }
     *
     * void action_video_volume_changed(lv_event_t* e) {
     *     lv_obj_t* slider = lv_event_get_target(e);
     *     int value = lv_slider_get_value(slider);
     *     app::VideoApp::getInstance().setVolume(value / 100.0f);
     * }
     *
     * void action_video_progress_changed(lv_event_t* e) {
     *     lv_obj_t* slider = lv_event_get_target(e);
     *     int value = lv_slider_get_value(slider);  // 0-100
     *     auto& app = app::VideoApp::getInstance();
     *     double duration = app.getDuration();
     *     if (duration > 0) {
     *         app.seek(duration * value / 100.0);
     *     }
     * }
     *
     * // 设置回调更新 UI
     * app::VideoApp::getInstance().setOnFrameUpdate([]() {
     *     // LVGL 需要刷新显示
     *     lv_refr_now(nullptr);
     * });
     *
     * app::VideoApp::getInstance().setOnProgressChanged([](const auto& info) {
     *     lv_slider_set_value(video_progress_slider, info.progressPercent * 100, LV_ANIM_OFF);
     *     lv_label_set_text_fmt(video_time_label, "%d:%02d / %d:%02d",
     *         (int)info.currentPosition / 60, (int)info.currentPosition % 60,
     *         (int)info.duration / 60, (int)info.duration % 60);
     * });
     * @endcode
     */
    class VideoApp
    {
    public:
        // ========== 单例访问 ==========
        /**
         * @brief 获取单例实例
         * @return VideoApp 单例引用
         */
        static VideoApp& getInstance();

        // ========== 显示区域设置 ==========

        /**
         * @brief 显示区域结构
         */
        struct DisplayArea
        {
            int x;      ///< 起始 X 坐标
            int y;      ///< 起始 Y 坐标
            int width;  ///< 宽度
            int height; ///< 高度
        };

        /**
         * @brief 设置视频显示区域
         * @param x 起始 X 坐标
         * @param y 起始 Y 坐标
         * @param width 宽度
         * @param height 高度
         */
        void setDisplayArea(int x, int y, int width, int height);

        /**
         * @brief 获取当前显示区域
         * @return 显示区域结构
         */
        [[nodiscard]] DisplayArea getDisplayArea() const;

        /**
         * @brief 设置 LVGL canvas 控件用于渲染
         * @param canvas LVGL canvas 对象指针
         */
        void setVideoCanvas(void* canvas);

        // ========== 播放控制 ==========

        /**
         * @brief 播放视频文件
         * @param filePath 视频文件路径
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
         * @brief 跳转到指定位置
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
         * @brief 获取视频总时长（秒）
         * @return 总时长，0 表示无播放或无法获取
         */
        [[nodiscard]] double getDuration() const;

        /**
         * @brief 获取播放进度百分比 [0.0, 1.0]
         * @return 进度百分比
         */
        [[nodiscard]] float getProgressPercent() const;

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
         * @brief 帧更新回调类型 - 通知 UI 需要刷新显示
         */
        using FrameUpdateCallback = std::function<void()>;

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
         * @brief 设置帧更新回调
         * @param callback 回调函数
         *
         * 当新帧解码完成时调用，UI 应调用 lv_refr_now() 刷新显示
         */
        void setOnFrameUpdate(FrameUpdateCallback callback);

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
        VideoApp();
        ~VideoApp();

        // 禁止拷贝
        VideoApp(const VideoApp&) = delete;
        VideoApp& operator=(const VideoApp&) = delete;

        // 内部方法
        void setupVideoCallbacks();
        void notifyProgressChanged();
        void notifyStateChanged(bool playing);
        void notifyError(const std::string& msg);

        // 成员变量
        std::unique_ptr<hal::VideoPlayer> m_videoPlayer;
        DisplayArea m_displayArea{0, 0, 800, 480};
        float m_volume = 1.0f;

        // 回调
        std::mutex m_callbackMutex;
        ProgressCallback m_onProgressChanged;
        FrameUpdateCallback m_onFrameUpdate;
        StateChangedCallback m_onStateChanged;
        ErrorCallback m_onError;
    };

} // namespace app