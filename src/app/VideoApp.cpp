#include "app/VideoApp.h"
#include "hal/VideoPlayer.h"
#include <algorithm>
#include <iostream>

namespace app
{

    // ==========================================
    // 构造与析构
    // ==========================================
    VideoApp::VideoApp()
        : m_videoPlayer(std::make_unique<hal::VideoPlayer>())
    {
        setupVideoCallbacks();
    }

    VideoApp::~VideoApp()
    {
        stop();
    }

    // ==========================================
    // 单例访问
    // ==========================================
    VideoApp& VideoApp::getInstance()
    {
        static VideoApp instance;
        return instance;
    }

    // ==========================================
    // 显示区域设置
    // ==========================================
    void VideoApp::setDisplayArea(int x, int y, int width, int height)
    {
        m_displayArea.x = x;
        m_displayArea.y = y;
        m_displayArea.width = width;
        m_displayArea.height = height;
        m_videoPlayer->setDisplayArea(x, y, width, height);
    }

    VideoApp::DisplayArea VideoApp::getDisplayArea() const
    {
        return m_displayArea;
    }

    void VideoApp::setVideoCanvas(void* canvas)
    {
        // VideoPlayer 内部使用 lv_obj_t*，这里做类型转换
        m_videoPlayer->setDisplayArea(m_displayArea.x, m_displayArea.y, m_displayArea.width, m_displayArea.height);
        // 注意：VideoPlayer 需要额外接口来设置 canvas，这里暂时通过 displayArea 间接设置
    }

    // ==========================================
    // 播放控制
    // ==========================================
    bool VideoApp::play(const std::string& filePath)
    {
        bool success = m_videoPlayer->play(filePath);

        if (success)
        {
            notifyStateChanged(true);
        }

        return success;
    }

    void VideoApp::pause()
    {
        m_videoPlayer->pause();
        notifyStateChanged(false);
    }

    void VideoApp::resume()
    {
        m_videoPlayer->resume();
        notifyStateChanged(true);
    }

    void VideoApp::stop()
    {
        m_videoPlayer->stop();
        notifyStateChanged(false);
    }

    void VideoApp::seek(double positionSeconds)
    {
        m_videoPlayer->seek(positionSeconds);
    }

    // ==========================================
    // 音量控制
    // ==========================================
    void VideoApp::setVolume(float volume)
    {
        m_volume = std::clamp(volume, 0.0f, 1.0f);
        m_videoPlayer->setVolume(m_volume);
    }

    float VideoApp::getVolume() const
    {
        return m_volume;
    }

    // ==========================================
    // 状态查询
    // ==========================================
    bool VideoApp::isPlaying() const
    {
        return m_videoPlayer->isPlaying();
    }

    bool VideoApp::isPaused() const
    {
        return m_videoPlayer->isPaused();
    }

    double VideoApp::getCurrentPosition() const
    {
        auto pos = m_videoPlayer->getCurrentPosition();
        return pos.value_or(0.0);
    }

    double VideoApp::getDuration() const
    {
        auto dur = m_videoPlayer->getDuration();
        return dur.value_or(0.0);
    }

    float VideoApp::getProgressPercent() const
    {
        double duration = getDuration();
        if (duration <= 0.0)
        {
            return 0.0f;
        }
        return static_cast<float>(getCurrentPosition() / duration);
    }

    // ==========================================
    // UI 回调设置
    // ==========================================
    void VideoApp::setOnProgressChanged(ProgressCallback callback)
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        m_onProgressChanged = std::move(callback);
    }

    void VideoApp::setOnFrameUpdate(FrameUpdateCallback callback)
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        m_onFrameUpdate = std::move(callback);
    }

    void VideoApp::setOnStateChanged(StateChangedCallback callback)
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        m_onStateChanged = std::move(callback);
    }

    void VideoApp::setOnError(ErrorCallback callback)
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        m_onError = std::move(callback);
    }

    // ==========================================
    // 内部私有方法
    // ==========================================
    void VideoApp::setupVideoCallbacks()
    {
        // 播放完成回调
        m_videoPlayer->setOnPlaybackComplete([this]() { notifyStateChanged(false); });

        // 帧更新回调
        m_videoPlayer->setOnFrameUpdate(
            [this]()
            {
                std::lock_guard<std::mutex> lock(m_callbackMutex);
                if (m_onFrameUpdate)
                {
                    m_onFrameUpdate();
                }
            });

        // 错误回调
        m_videoPlayer->setOnError([this](const std::string& msg) { notifyError(msg); });
    }

    void VideoApp::notifyProgressChanged()
    {
        ProgressInfo info;
        info.currentPosition = getCurrentPosition();
        info.duration = getDuration();
        info.progressPercent = getProgressPercent();

        std::lock_guard<std::mutex> lock(m_callbackMutex);
        if (m_onProgressChanged)
        {
            m_onProgressChanged(info);
        }
    }

    void VideoApp::notifyStateChanged(bool playing)
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        if (m_onStateChanged)
        {
            m_onStateChanged(playing);
        }
    }

    void VideoApp::notifyError(const std::string& msg)
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        if (m_onError)
        {
            m_onError(msg);
        }
        else
        {
            std::cerr << "[VideoApp Error] " << msg << std::endl;
        }
    }

} // namespace app