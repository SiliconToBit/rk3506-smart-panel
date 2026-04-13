#include "app/MusicApp.h"
#include "hal/AudioPlayer.h"
#include <algorithm>
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
        stopProgressTimer();
        lv_async_call_cancel(asyncSongChangedThunk, this);
        lv_async_call_cancel(asyncStateChangedThunk, this);
        lv_async_call_cancel(asyncErrorThunk, this);
        lv_async_call_cancel(asyncAutoNextThunk, this);
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
        std::vector<std::string> scannedPlaylist;
        std::string currentSongPath;
        const bool hasCurrentSong = (m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_playlist.size()));
        if (hasCurrentSong)
        {
            currentSongPath = m_playlist[static_cast<size_t>(m_currentIndex)];
        }

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
                        scannedPlaylist.push_back(filePath);
                    }
                }
            }

            // 按文件名排序
            std::sort(scannedPlaylist.begin(), scannedPlaylist.end());

            m_playlist = std::move(scannedPlaylist);
            m_currentIndex = -1;

            if (!currentSongPath.empty())
            {
                auto songIter = std::find(m_playlist.begin(), m_playlist.end(), currentSongPath);
                if (songIter != m_playlist.end())
                {
                    m_currentIndex = static_cast<int>(songIter - m_playlist.begin());
                }
            }

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
        bool wasPlaying = m_audioPlayer->isPlaying();
        if (wasPlaying)
        {
            m_audioPlayer->stop();
        }
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
            loadLyricsForCurrentSong();
            notifySongChanged();
            notifyStateChanged(true);
            notifyLyricChanged(0.0);
        }

        return success;
    }

    bool MusicApp::play(const std::string& filePath)
    {
        // 检查是否已在列表中
        auto songIter = std::find(m_playlist.begin(), m_playlist.end(), filePath);
        if (songIter != m_playlist.end())
        {
            return play(static_cast<int>(songIter - m_playlist.begin()));
        }

        // 添加到列表并播放
        m_playlist.push_back(filePath);
        m_currentIndex = static_cast<int>(m_playlist.size()) - 1;

        bool success = m_audioPlayer->play(filePath);
        if (success)
        {
            loadLyricsForCurrentSong();
            notifySongChanged();
            notifyStateChanged(true);
            notifyLyricChanged(0.0);
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
        {
            std::lock_guard<std::mutex> lock(m_lyricMutex);
            m_lyricParser.clear();
            m_lastLyricText.clear();
        }

        LyricChangedCallback onLyricChanged;
        {
            std::lock_guard<std::mutex> lock(m_callbackMutex);
            onLyricChanged = m_onLyricChanged;
        }

        if (onLyricChanged)
        {
            onLyricChanged("");
        }
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

        // 播放超过3秒时，按上一首回到当前歌曲开头
        if (getCurrentPosition() > 3.0)
        {
            seek(0.0);
            return true;
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
        if (m_audioPlayer->seek(positionSeconds))
        {
            notifyLyricChanged(positionSeconds);
        }
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

    void MusicApp::setOnLyricChanged(LyricChangedCallback callback)
    {
        {
            std::lock_guard<std::mutex> lock(m_callbackMutex);
            m_onLyricChanged = std::move(callback);
        }

        notifyLyricChanged(getCurrentPosition());
    }

    // ==========================================
    // 内部私有方法
    // ==========================================
    void MusicApp::setupAudioCallbacks()
    {
        // 播放完成后仅投递主线程任务，避免在解码线程直接切歌
        m_audioPlayer->setOnPlaybackComplete(
            [this]()
            {
                if (m_autoNextPending.exchange(true))
                {
                    return;
                }

                if (lv_async_call(asyncAutoNextThunk, this) != LV_RES_OK)
                {
                    m_autoNextPending = false;
                }
            });

        // 错误回调
        m_audioPlayer->setOnError([this](const std::string& msg) { notifyError(msg); });
    }

    void MusicApp::notifyProgressChanged()
    {
        if (!isPlaying() || isPaused())
        {
            return;
        }

        ProgressInfo info;
        info.currentPosition = getCurrentPosition();
        info.duration = getDuration();
        info.progressPercent = getProgressPercent();
        notifyLyricChanged(info.currentPosition);

        std::lock_guard<std::mutex> lock(m_callbackMutex);
        if (m_onProgressChanged)
        {
            m_onProgressChanged(info);
        }
    }

    void MusicApp::notifySongChanged()
    {
        if (m_songChangedPending.exchange(true))
        {
            return;
        }

        {
            std::lock_guard<std::mutex> lock(m_pendingSongMutex);
            m_pendingSongData.index = m_currentIndex;
            m_pendingSongData.songName = getCurrentSongName();
        }

        if (lv_async_call(asyncSongChangedThunk, this) != LV_RES_OK)
        {
            m_songChangedPending = false;
        }
    }

    void MusicApp::notifyStateChanged(bool playing)
    {
        if (m_lastNotifiedState.load() == playing)
        {
            return;
        }

        if (m_stateChangedPending.exchange(true))
        {
            return;
        }

        {
            std::lock_guard<std::mutex> lock(m_pendingStateMutex);
            m_pendingStateData.isPlaying = playing;
        }

        if (lv_async_call(asyncStateChangedThunk, this) != LV_RES_OK)
        {
            m_stateChangedPending = false;
            return;
        }

        m_lastNotifiedState = playing;
    }

    void MusicApp::notifyError(const std::string& msg)
    {
        if (m_errorPending.exchange(true))
        {
            return;
        }

        {
            std::lock_guard<std::mutex> lock(m_pendingErrorMutex);
            m_pendingErrorData.errorMsg = msg;
        }

        if (lv_async_call(asyncErrorThunk, this) != LV_RES_OK)
        {
            m_errorPending = false;
            std::cerr << "[MusicApp Error] " << msg << '\n';
        }
    }

    void MusicApp::loadLyricsForCurrentSong()
    {
        std::lock_guard<std::mutex> lock(m_lyricMutex);
        m_lyricParser.clear();
        m_lastLyricText.clear();

        if (m_currentIndex < 0 || m_currentIndex >= static_cast<int>(m_playlist.size()))
        {
            return;
        }

        m_lyricParser.loadForAudioFile(m_playlist[m_currentIndex]);
    }

    void MusicApp::notifyLyricChanged(double positionSeconds)
    {
        std::string lyricText;
        {
            std::lock_guard<std::mutex> lock(m_lyricMutex);
            lyricText = m_lyricParser.getLyricAt(positionSeconds);
            if (lyricText == m_lastLyricText)
            {
                return;
            }
            m_lastLyricText = lyricText;
        }

        LyricChangedCallback onLyricChanged;
        {
            std::lock_guard<std::mutex> lock(m_callbackMutex);
            onLyricChanged = m_onLyricChanged;
        }

        if (onLyricChanged)
        {
            onLyricChanged(lyricText);
        }
    }

    // ==========================================
    // LVGL 异步回调 Thunk
    // ==========================================
    void MusicApp::asyncSongChangedThunk(void* userData)
    {
        auto* self = static_cast<MusicApp*>(userData);
        self->m_songChangedPending = false;

        int index;
        std::string songName;
        {
            std::lock_guard<std::mutex> lock(self->m_pendingSongMutex);
            index = self->m_pendingSongData.index;
            songName = std::move(self->m_pendingSongData.songName);
        }

        std::lock_guard<std::mutex> lock(self->m_callbackMutex);
        if (self->m_onSongChanged)
        {
            self->m_onSongChanged(index, songName);
        }
    }

    void MusicApp::asyncStateChangedThunk(void* userData)
    {
        auto* self = static_cast<MusicApp*>(userData);
        self->m_stateChangedPending = false;

        bool isPlaying;
        {
            std::lock_guard<std::mutex> lock(self->m_pendingStateMutex);
            isPlaying = self->m_pendingStateData.isPlaying;
        }

        std::lock_guard<std::mutex> lock(self->m_callbackMutex);
        if (self->m_onStateChanged)
        {
            self->m_onStateChanged(isPlaying);
        }
    }

    void MusicApp::asyncErrorThunk(void* userData)
    {
        auto* self = static_cast<MusicApp*>(userData);
        self->m_errorPending = false;

        std::string errorMsg;
        {
            std::lock_guard<std::mutex> lock(self->m_pendingErrorMutex);
            errorMsg = std::move(self->m_pendingErrorData.errorMsg);
        }

        std::lock_guard<std::mutex> lock(self->m_callbackMutex);
        if (self->m_onError)
        {
            self->m_onError(errorMsg);
        }
        else
        {
            std::cerr << "[MusicApp Error] " << errorMsg << '\n';
        }
    }

    void MusicApp::asyncAutoNextThunk(void* userData)
    {
        auto* self = static_cast<MusicApp*>(userData);
        self->m_autoNextPending = false;

        if (self->m_playMode == PlayMode::SingleLoop)
        {
            if (self->m_currentIndex >= 0)
            {
                self->play(self->m_currentIndex);
            }
            return;
        }

        self->next();
    }

    void MusicApp::progressTimerCallback(lv_timer_t* timer)
    {
        auto* self = static_cast<MusicApp*>(timer->user_data);
        self->notifyProgressChanged();
    }

    void MusicApp::startProgressTimer(uint32_t periodMs)
    {
        if (m_progressTimer != nullptr)
        {
            lv_timer_del(m_progressTimer);
        }
        m_progressTimer = lv_timer_create(progressTimerCallback, periodMs, this);
    }

    void MusicApp::stopProgressTimer()
    {
        if (m_progressTimer != nullptr)
        {
            lv_timer_del(m_progressTimer);
            m_progressTimer = nullptr;
        }
    }

    std::string MusicApp::extractSongName(const std::string& filePath) const
    {
        fs::path path(filePath);
        std::string filename = path.filename().string();

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

        fs::path path(filePath);
        std::string ext = path.extension().string();

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
                if (m_playlist.size() == 1)
                {
                    return 0;
                }
                std::lock_guard<std::mutex> lock(m_rngMutex);
                std::uniform_int_distribution<int> dist(0, static_cast<int>(m_playlist.size()) - 1);
                int newIndex = dist(m_rng);
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
                    return -1;
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
                std::lock_guard<std::mutex> lock(m_rngMutex);
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
                    return -1;
                }
                return prevIdx;
            }
        }
    }

} // namespace app
