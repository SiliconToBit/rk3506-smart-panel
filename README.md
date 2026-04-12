# RK3506 Smart Panel

基于 RK3506 平台的智能面板应用，使用 LVGL 8 构建现代化嵌入式 GUI 界面。

## 功能特性

- **天气应用** - 实时天气显示与多日预报
- **音乐播放器** - 支持多种音频格式的本地音乐播放
- **视频播放器** - 视频播放与显示控制
- **传感器监测** - DHT11 温湿度传感器、MPU6050 六轴传感器
- **智能家居控制** - LED 灯光控制、蜂鸣器控制

## 目录结构

```
rk3506-smart-panel/
├── CMakeLists.txt          # CMake 构建配置
├── README.md               # 项目说明文档
├── configs/                # 配置文件
│   └── AppConfig.h         # 应用配置（设备路径、API密钥等）
├── docs/                   # 文档
├── examples/               # 示例代码
├── include/                # 头文件
│   ├── main.h              # 主程序头文件
│   ├── app/                # 应用层模块
│   │   ├── CityData.h      # 城市数据管理
│   │   ├── MusicApp.h      # 音乐播放应用
│   │   ├── VideoApp.h      # 视频播放应用
│   │   └── WeatherApp.h    # 天气应用
│   ├── hal/                # 硬件抽象层
│   │   ├── AudioPlayer.h   # 音频播放器
│   │   ├── CpuAffinity.h   # CPU 亲和性管理
│   │   ├── DeviceNode.h    # 设备节点基类
│   │   ├── HttpClient.h    # HTTP 客户端
│   │   ├── ThreadPool.h    # 线程池
│   │   └── VideoPlayer.h   # 视频播放器
│   ├── sensor/             # 传感器驱动
│   │   ├── Buzzer.h        # 蜂鸣器驱动
│   │   ├── Dht11.h         # DHT11 温湿度传感器
│   │   └── Mpu6050.h       # MPU6050 六轴传感器
│   └── fonts/              # 字体资源
│       └── qweather_icons.h # 和风天气图标字体
├── src/                    # 源文件
│   ├── main.cpp            # 主程序入口
│   ├── app/                # 应用层实现
│   ├── hal/                # 硬件抽象层实现
│   ├── sensor/             # 传感器驱动实现
│   └── ui/                 # UI 代码（EEZ Studio 生成）
├── tests/                  # 测试代码
└── third_party/            # 第三方库
    └── rk/                 # RK 平台相关库
        ├── common/         # 通用组件
        ├── lvgl8/          # LVGL 8 适配
        └── sys/            # 系统接口
```

## 环境要求

### 开发环境

- **CMake** >= 3.10
- **C 标准**: C11
- **C++ 标准**: C++17
- **交叉编译工具链**: arm-buildroot-linux-gnueabihf

### 依赖库

- **LVGL 8** - 嵌入式图形库
- **FFmpeg** - 音视频解码
- **ALSA** - 音频输出
- **cURL** - HTTP 请求
- **pthread** - 多线程支持

### 目标平台

- **SoC**: Rockchip RK3506
- **OS**: Buildroot Linux
- **Display**: 800x480 LCD 面板

## 编译说明

### 设置环境变量

```bash
export RK3506_TOOLCHAIN_DIR=/path/to/buildroot/output/rockchip_rk3506_luckfox/host
```

### 编译步骤

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

生成的可执行文件位于 `build/rk3506_smart_panel`。

### 清理构建

```bash
make distclean
```

## 配置说明

### 设备路径配置

编辑 `configs/AppConfig.h` 修改设备路径：

```c
#define APP_DEV_DHT11 "/dev/dht11"        // DHT11 传感器
#define APP_DEV_MPU6050 "/dev/mpu6050"    // MPU6050 传感器
#define APP_GPIO_LED1 "/dev/led1"         // LED1
#define APP_GPIO_LED2 "/dev/led2"         // LED2
#define APP_GPIO_BUZZER "/dev/buzzer"     // 蜂鸣器
```

### 天气 API 配置

在 `configs/AppConfig.h` 中配置和风天气 API：

```c
#define APP_WEATHER_API_HOST "your-api-host"
#define APP_WEATHER_API_KEY "your-api-key"
#define APP_DEFAULT_CITY "城市名"
```

### MQTT 配置

```c
#define APP_MQTT_HOST "192.168.1.87"
#define APP_MQTT_PORT 1883
#define APP_MQTT_CLIENT_ID "rk3506_device"
```

## 模块介绍

### 应用层 (app)

| 模块 | 说明 |
|------|------|
| `WeatherApp` | 天气数据获取与显示，支持实时天气和预报 |
| `MusicApp` | 音乐播放管理，支持播放列表和播放控制 |
| `VideoApp` | 视频播放管理，支持 LVGL 显示区域集成 |
| `CityData` | 城市数据管理，用于天气查询 |

### 硬件抽象层 (hal)

| 模块 | 说明 |
|------|------|
| `AudioPlayer` | 基于 FFmpeg + ALSA 的音频播放器 ([详细设计](docs/AudioPlayer_Design.md)) |
| `VideoPlayer` | 视频播放器封装 |
| `HttpClient` | HTTP 客户端，使用线程池管理异步请求 |
| `CpuAffinity` | CPU 亲和性管理工具 |
| `ThreadPool` | 轻量级线程池实现 |
| `DeviceNode` | 字符设备节点基类 |

### 传感器层 (sensor)

| 模块 | 说明 |
|------|------|
| `Dht11` | DHT11 温湿度传感器驱动 |
| `Mpu6050` | MPU6050 六轴传感器驱动 |
| `Buzzer` | 蜂鸣器控制 |

## 技术栈

- **GUI 框架**: LVGL 8.x
- **UI 设计**: EEZ Studio
- **编程语言**: C++17 / C11
- **构建系统**: CMake
- **音视频**: FFmpeg + ALSA
- **网络**: cURL
- **多线程**: pthread + std::thread + 线程池

## 架构设计

```
┌─────────────────────────────────────────────────────┐
│                    UI Layer (LVGL)                   │
│                   src/ui (EEZ Studio)                │
├─────────────────────────────────────────────────────┤
│                   Application Layer                  │
│     WeatherApp  │  MusicApp  │  VideoApp  │  ...   │
├─────────────────────────────────────────────────────┤
│              Hardware Abstraction Layer              │
│    AudioPlayer  │  VideoPlayer  │  HttpClient  │    │
│              ThreadPool  │  CpuAffinity             │
├─────────────────────────────────────────────────────┤
│                   Driver Layer                       │
│      DHT11  │  MPU6050  │  Buzzer  │  LED  │  ...   │
├─────────────────────────────────────────────────────┤
│                   Linux Kernel                       │
│              /dev/dht11, /dev/mpu6050, ...           │
└─────────────────────────────────────────────────────┘
```

## 线程模型

### 线程与 CPU 亲和性

| 线程 | CPU 绑定 | 说明 |
|------|----------|------|
| 主线程 (UI) | CPU1 | LVGL 刷新循环 |
| 音频解码 | CPU0 | FFmpeg 解码 + ALSA 输出 |
| 视频解码 | CPU2 | FFmpeg 视频解码 |
| 视频内置音频 | CPU2 | 与视频解码同核心 |
| HTTP 异步请求 | CPU0 | 线程池工作线程 (2个) |
| LVGL 输入设备 | 不绑定 | 触摸/按键事件读取 |

### 线程池

HTTP 异步请求使用线程池管理，避免频繁创建/销毁线程：

```cpp
// 线程池配置
ThreadPool(2, selectHttpCpu(), "HttpWorker")
// 2个工作线程，绑定到 CPU0，线程名称前缀 "HttpWorker"
```

### CPU 亲和性工具

使用 `hal::CpuAffinity` 统一管理线程 CPU 绑定：

```cpp
// 绑定 std::thread
hal::bindThreadToCpu(thread, cpuId, "ThreadName");

// 绑定当前线程（用于 std::async 内部）
hal::bindCurrentThreadToCpu(cpuId, "ThreadName");

// 设置线程优先级
hal::setThreadPriority(thread, SCHED_FIFO, priority, "ThreadName");

// 获取推荐 CPU
hal::selectAudioCpu();   // 音频推荐 CPU0
hal::selectVideoCpu();   // 视频推荐 CPU2 (3核+)
hal::selectUiCpu();      // UI 推荐 CPU1
hal::selectHttpCpu();    // HTTP 推荐 CPU0
```

## 注意事项

1. **交叉编译**: 本项目需要使用 RK3506 交叉编译工具链
2. **设备节点**: 确保内核驱动已正确加载并创建设备节点
3. **资源文件**: 音乐文件默认放在 `/root/Music` 目录
4. **网络连接**: 天气功能需要网络连接

## 许可证

Copyright (c) 2024 RK3506 Smart Panel Project
