#include "app/MusicApp.h"
#include "hal/AudioPlayer.h"
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <random>

namespace fs = std::filesystem;

namespace app
{

    // ==========================================
    // 构造与析构
    // ==========================================
    MusicApp::MusicApp()
        : m_audioPlayer(std::make_unique<hal::AudioPlayer>())
        , m_rng(std::random_device{}())
    {
        setupAudioCallbacks();
    }

    MusicApp::~MusicApp()
    {
        stop();
    }

    // ==========================================
    // 单例访问
    // ==========================================
    MusicApp& MusicApp::getInstance()
    {
        static MusicApp instance;
        return instance;
    }

    // ==========================================
    // 播放列表管理
    // ==========================================
    int MusicApp::loadDirectory(const std::string& directoryPath)
    {
        clearPlaylist();

        try
        {
            if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath))
            {
                notifyError("Directory not found: " + directoryPath);
                return 0;
            }

            for (const auto& entry : fs::directory_iterator(directoryPath))
            {
                if (entry.is_regular_file())
                {
                    std::string filePath = entry.path().string();
                    if (isAudioFile(filePath))
                    {
                        m_playlist.push_back(filePath);
                    }
                }
            }

            // 按文件名排序
            std::sort(m_playlist.begin(), m_playlist.end());

            return static_cast<int>(m_playlist.size());
        }
        catch (const fs::filesystem_error& e)
        {
            notifyError("Failed to scan directory: " + std::string(e.what()));
            return 0;
        }
    }

    void MusicApp::clearPlaylist()
    {
        stop();
        m_playlist.clear();
        m_currentIndex = -1;
    }

    std::vector<std::string> MusicApp::getPlaylist() const
    {
        return m_playlist;
    }

    int MusicApp::getPlaylistSize() const
    {
        return static_cast<int>(m_playlist.size());
    }

    int MusicApp::getCurrentIndex() const
    {
        return m_currentIndex;
    }

    std::string MusicApp::getCurrentSongName() const
    {
        if (m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_playlist.size()))
        {
            return extractSongName(m_playlist[m_currentIndex]);
        }
        return "";
    }

    std::string MusicApp::getSongName(int index) const
    {
        if (index >= 0 && index < static_cast<int>(m_playlist.size()))
        {
            return extractSongName(m_playlist[index]);
        }
        return "";
    }

    // ==========================================
    // 播放控制
    // ==========================================
    bool MusicApp::play(int index)
    {
        if (index < 0 || index >= static_cast<int>(m_playlist.size()))
        {
            notifyError("Invalid playlist index");
            return false;
        }

        m_currentIndex = index;
        bool success = m_audioPlayer->play(m_playlist[index]);

        if (success)
        {
            notifySongChanged();
            notifyStateChanged(true);
        }

        return success;
    }

    bool MusicApp::play(const std::string& filePath)
    {
        // 检查是否已在列表中
        auto it = std::find(m_playlist.begin(), m_playlist.end(), filePath);
        if (it != m_playlist.end())
        {
            return play(static_cast<int>(it - m_playlist.begin()));
        }

        // 添加到列表并播放
        m_playlist.push_back(filePath);
        m_currentIndex = static_cast<int>(m_playlist.size()) - 1;

        bool success = m_audioPlayer->play(filePath);
        if (success)
        {
            notifySongChanged();
            notifyStateChanged(true);
        }

        return success;
    }

    void MusicApp::pause()
    {
        m_audioPlayer->pause();
        notifyStateChanged(false);
    }

    void MusicApp::resume()
    {
        m_audioPlayer->resume();
        notifyStateChanged(true);
    }

    void MusicApp::stop()
    {
        m_audioPlayer->stop();
        notifyStateChanged(false);
    }

    bool MusicApp::next()
    {
        if (m_playlist.empty())
        {
            return false;
        }

        int nextIndex = getNextIndex();
        if (nextIndex < 0)
        {
            // 已到末尾，根据播放模式决定行为
            if (m_playMode == PlayMode::Loop)
            {
                nextIndex = 0;
            }
            else
            {
                stop();
                return false;
            }
        }

        return play(nextIndex);
    }

    bool MusicApp::prev()
    {
        if (m_playlist.empty())
        {
            return false;
        }

        int prevIndex = getPrevIndex();
        if (prevIndex < 0)
        {
            // 已在开头
            if (m_playMode == PlayMode::Loop)
            {
                prevIndex = static_cast<int>(m_playlist.size()) - 1;
            }
            else
            {
                return false;
            }
        }

        return play(prevIndex);
    }

    void MusicApp::seek(double positionSeconds)
    {
        m_audioPlayer->seek(positionSeconds);
    }

    // ==========================================
    // 音量控制
    // ==========================================
    void MusicApp::setVolume(float volume)
    {
        m_volume = std::clamp(volume, 0.0f, 1.0f);
        m_audioPlayer->setVolume(m_volume);
    }

    float MusicApp::getVolume() const
    {
        return m_volume;
    }

    // ==========================================
    // 状态查询
    // ==========================================
    bool MusicApp::isPlaying() const
    {
        return m_audioPlayer->isPlaying();
    }

    bool MusicApp::isPaused() const
    {
        return m_audioPlayer->isPaused();
    }

    double MusicApp::getCurrentPosition() const
    {
        auto pos = m_audioPlayer->getCurrentPosition();
        return pos.value_or(0.0);
    }

    double MusicApp::getDuration() const
    {
        auto dur = m_audioPlayer->getDuration();
        return dur.value_or(0.0);
    }

    float MusicApp::getProgressPercent() const
    {
        double duration = getDuration();
        if (duration <= 0.0)
        {
            return 0.0f;
        }
        return static_cast<float>(getCurrentPosition() / duration);
    }

    // ==========================================
    // 播放模式
    // ==========================================
    void MusicApp::setPlayMode(PlayMode mode)
    {
        m_playMode = mode;
    }

    MusicApp::PlayMode MusicApp::getPlayMode() const
    {
        return m_playMode;
    }

    // ==========================================
    // UI 回调设置
    // ==========================================
    void MusicApp::setOnProgressChanged(ProgressCallback callback)
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        m_onProgressChanged = std::move(callback);
    }

    void MusicApp::setOnSongChanged(SongChangedCallback callback)
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        m_onSongChanged = std::move(callback);
    }

    void MusicApp::setOnStateChanged(StateChangedCallback callback)
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        m_onStateChanged = std::move(callback);
    }

    void MusicApp::setOnError(ErrorCallback callback)
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        m_onError = std::move(callback);
    }

    // ==========================================
    // 内部私有方法
    // ==========================================
    void MusicApp::setupAudioCallbacks()
    {
        // 播放完成时自动播放下一首
        m_audioPlayer->setOnPlaybackComplete(
            [this]()
            {
                if (m_playMode == PlayMode::SingleLoop)
                {
                    // 单曲循环，重新播放当前歌曲
                    if (m_currentIndex >= 0)
                    {
                        play(m_currentIndex);
                    }
                }
                else
                {
                    next();
                }
            });

        // 错误回调
        m_audioPlayer->setOnError([this](const std::string& msg) { notifyError(msg); });
    }

    void MusicApp::notifyProgressChanged()
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

    void MusicApp::notifySongChanged()
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        if (m_onSongChanged)
        {
            m_onSongChanged(m_currentIndex, getCurrentSongName());
        }
    }

    void MusicApp::notifyStateChanged(bool playing)
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        if (m_onStateChanged)
        {
            m_onStateChanged(playing);
        }
    }

    void MusicApp::notifyError(const std::string& msg)
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        if (m_onError)
        {
            m_onError(msg);
        }
        else
        {
            std::cerr << "[MusicApp Error] " << msg << std::endl;
        }
    }

    std::string MusicApp::extractSongName(const std::string& filePath) const
    {
        fs::path p(filePath);
        std::string filename = p.filename().string();

        // 移除扩展名
        size_t dotPos = filename.find_last_of('.');
        if (dotPos != std::string::npos)
        {
            filename = filename.substr(0, dotPos);
        }

        return filename;
    }

    bool MusicApp::isAudioFile(const std::string& filePath) const
    {
        static const std::vector<std::string> audioExtensions = {".mp3", ".wav", ".flac", ".aac",
                                                                 ".ogg", ".m4a", ".wma",  ".ape"};

        fs::path p(filePath);
        std::string ext = p.extension().string();

        // 转换为小写
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        return std::find(audioExtensions.begin(), audioExtensions.end(), ext) != audioExtensions.end();
    }

    int MusicApp::getNextIndex() const
    {
        if (m_playlist.empty())
        {
            return -1;
        }

        switch (m_playMode)
        {
            case PlayMode::Shuffle:
            {
                // 随机选择一个不同的索引
                if (m_playlist.size() == 1)
                {
                    return 0;
                }
                std::uniform_int_distribution<int> dist(0, static_cast<int>(m_playlist.size()) - 1);
                int newIndex = dist(m_rng);
                // 确保不是当前歌曲
                while (newIndex == m_currentIndex)
                {
                    newIndex = dist(m_rng);
                }
                return newIndex;
            }
            case PlayMode::Sequence:
            case PlayMode::Loop:
            case PlayMode::SingleLoop:
            default:
            {
                int nextIdx = m_currentIndex + 1;
                if (nextIdx >= static_cast<int>(m_playlist.size()))
                {
                    return -1; // 已到末尾
                }
                return nextIdx;
            }
        }
    }

    int MusicApp::getPrevIndex() const
    {
        if (m_playlist.empty())
        {
            return -1;
        }

        switch (m_playMode)
        {
            case PlayMode::Shuffle:
            {
                // 随机选择
                std::uniform_int_distribution<int> dist(0, static_cast<int>(m_playlist.size()) - 1);
                return dist(m_rng);
            }
            case PlayMode::Sequence:
            case PlayMode::Loop:
            case PlayMode::SingleLoop:
            default:
            {
                int prevIdx = m_currentIndex - 1;
                if (prevIdx < 0)
                {
                    return -1; // 已在开头
                }
                return prevIdx;
            }
        }
    }

} // namespace app
