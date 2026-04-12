# AudioPlayer 设计说明

## 概述

`AudioPlayer` 是一个基于 FFmpeg + ALSA 的音频播放器类，位于 `hal` 命名空间中，专为 RK3506 嵌入式平台设计。支持常见音频格式（MP3/AAC/FLAC/WAV 等）的解码播放，提供完整的播放控制功能。

## 架构设计

### 整体架构

```
+------------------------------------------+
|          AudioPlayer (公开接口)           |
|  play() / stop() / pause() / resume()    |
|  seek() / setVolume() / 状态查询          |
+-------------------+----------------------+
                    |
                    v
+------------------------------------------+
|       decodeLoop() (独立解码线程)         |
|  读取文件 -> 解码 -> 重采样 -> 写入ALSA    |
+-------------------+----------------------+
                    |
        +-----------+-----------+
        v                       v
+---------------+       +--------------+
|   FFmpeg 层   |       |   ALSA 层    |
|  解封装/解码   |       |  PCM 输出    |
+---------------+       +--------------+
```

### 音频处理管道

```
音频文件 (MP3/AAC/FLAC...)
    |
    v
[AVFormatContext] 解封装 -> AVPacket
    |
    v
[AVCodecContext] 解码 -> AVFrame (原始 PCM)
    |
    v
[SwrContext] 重采样 -> S16, 44.1kHz, 立体声
    |
    v
[软件音量控制] 乘以音量系数 (0.0 ~ 1.0)
    |
    v
[ALSA snd_pcm_writei] 输出到声卡设备
```

## 核心设计模式

### 1. RAII 资源管理

使用自定义删除器的 `std::unique_ptr` 管理 C 语言资源，确保异常安全和自动释放：

```cpp
template <typename T> struct FFmpegDeleter {
    void operator()(T* ptr) const noexcept {
        if constexpr (std::is_same_v<T, AVFormatContext>)
            avformat_close_input(&ptr);
        else if constexpr (std::is_same_v<T, AVCodecContext>)
            avcodec_free_context(&ptr);
        else if constexpr (std::is_same_v<T, SwrContext>)
            swr_free(&ptr);
        else if constexpr (std::is_same_v<T, snd_pcm_t>) {
            snd_pcm_drain(ptr);
            snd_pcm_close(ptr);
        }
    }
};

template <typename T> using FFmpegPtr = std::unique_ptr<T, FFmpegDeleter<T>>;
```

**管理的资源**：
- `AVFormatContext` - 封装格式上下文
- `AVCodecContext` - 解码器上下文
- `SwrContext` - 音频重采样上下文
- `snd_pcm_t` - ALSA PCM 设备句柄

### 2. 生产者-消费者模型

`decodeLoop()` 是核心解码循环，实现简化的生产者-消费者模式：

```
av_read_frame() -> avcodec_send_packet() -> avcodec_receive_frame()
    -> swr_convert() -> snd_pcm_writei()
```

### 3. 线程安全设计

| 变量类型 | 示例 | 保护机制 |
|---------|------|---------|
| 原子变量 | `m_isPlaying`, `m_volume`, `m_currentPts` | `std::atomic`，无锁访问 |
| 跨线程回调 | `m_onComplete`, `m_onError` | `std::mutex` 保护 |
| 暂停同步 | `m_isPaused`, `m_stopRequested` | `std::condition_variable` |

### 4. 暂停/恢复同步机制

使用条件变量替代轮询，实现零延迟的暂停/恢复响应：

```cpp
// 解码线程等待
bool AudioPlayer::waitForResumeOrStop() {
    std::unique_lock<std::mutex> lock(m_pauseMutex);
    m_pauseCondition.wait(lock, [this]() {
        return !m_isPaused.load() || m_stopRequested.load();
    });
    return !m_stopRequested.load();
}

// 主线程唤醒
void AudioPlayer::resume() {
    if (m_isPaused) {
        m_isPaused = false;
        notifyPauseCondition(); // 唤醒条件变量
    }
}
```

## 公开接口

### 播放控制

| 方法 | 说明 |
|------|------|
| `play(filePath)` | 开始播放指定音频文件 |
| `stop()` | 停止播放并释放资源 |
| `pause()` | 暂停播放 |
| `resume()` | 从暂停状态恢复 |
| `seek(positionSeconds)` | 跳转到指定播放位置 |

### 状态查询

| 方法 | 说明 |
|------|------|
| `isPlaying()` | 检查是否正在播放 |
| `isPaused()` | 检查是否处于暂停状态 |
| `getCurrentPosition()` | 获取当前播放进度（秒） |
| `getDuration()` | 获取音频总时长（秒） |

### 配置

| 方法 | 说明 |
|------|------|
| `setVolume(volume)` | 设置音量 (0.0 ~ 1.0) |
| `setCpuAffinity(cpuId)` | 设置解码线程 CPU 亲和性 |
| `setOnPlaybackComplete(callback)` | 设置播放完成回调 |
| `setOnError(callback)` | 设置错误回调 |

## 关键流程

### play() 流程

```
1. 如果正在播放，先 stop()
2. openFile(filePath)
   - 打开封装格式 (avformat_open_input)
   - 查找音频流
   - 初始化解码器 (avcodec_open2)
   - 初始化重采样器 (swr_init)
   - 打开 ALSA 设备 (snd_pcm_open)
3. 重置状态标志
4. 启动 decodeLoop 线程
5. 绑定 CPU 亲和性
```

### stop() 流程

```
1. m_stopRequested = true
2. notifyPauseCondition() (唤醒可能的暂停等待)
3. m_decodeThread.join() (等待线程结束)
4. cleanup() (智能指针自动释放资源)
```

### seek() 流程

```
1. 计算目标时间戳
2. av_seek_frame() 执行跳转
3. avcodec_flush_buffers() 刷新解码器缓冲区
4. snd_pcm_drop() + snd_pcm_prepare() 排空 ALSA 缓冲区
5. 更新 m_currentPts
```

## 技术特性

### 现代 C++ 特性

- **C++17**: `std::optional`, `std::clamp`, `if constexpr`
- **智能指针**: 自动资源管理
- **原子操作**: 无锁状态控制
- **条件变量**: 高效线程同步
- **移动语义**: 线程安全传递

### 性能优化

- **CPU 亲和性**: 解码线程绑定到指定 CPU 核心，减少缓存失效
- **无锁状态**: 使用 `std::atomic` 避免不必要的锁竞争
- **条件变量**: 暂停时阻塞线程，避免轮询浪费 CPU

### 错误处理

- **ALSA 恢复**: 写入失败时自动尝试恢复设备
- **回调通知**: 通过 `setOnError()` 上报错误信息
- **资源清理**: 析构时确保所有资源正确释放

## 使用示例

```cpp
#include "hal/AudioPlayer.h"

hal::AudioPlayer player;

// 配置
player.setCpuAffinity(2);  // 绑定到 CPU 2
player.setVolume(0.8f);    // 设置音量 80%

// 设置回调
player.setOnPlaybackComplete([]() {
    std::cout << "播放完成\n";
});

player.setOnOnError([](const std::string& msg) {
    std::cerr << "错误: " << msg << "\n";
});

// 播放控制
if (player.play("/path/to/music.mp3")) {
    // 获取信息
    if (auto duration = player.getDuration()) {
        std::cout << "总时长: " << *duration << " 秒\n";
    }

    // 暂停/恢复
    player.pause();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    player.resume();

    // Seek
    player.seek(30.0);  // 跳到 30 秒

    // 等待播放完成
    while (player.isPlaying()) {
        if (auto pos = player.getCurrentPosition()) {
            std::cout << "当前: " << *pos << " 秒\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// 停止
player.stop();
```

## 注意事项

1. **CPU 亲和性**: `setCpuAffinity()` 必须在 `play()` 之前调用
2. **回调线程**: 回调函数在解码线程中执行，应避免耗时操作
3. **单实例播放**: 同一实例一次只能播放一个文件
4. **Seek 限制**: 某些流媒体格式可能不支持 Seek
5. **音量范围**: 音量值超出 0.0~1.0 范围会被自动截断

## 依赖库

- **FFmpeg**: libavformat, libavcodec, libavutil, libswresample
- **ALSA**: libasound
- **C++ 标准库**: thread, mutex, atomic, condition_variable, functional
