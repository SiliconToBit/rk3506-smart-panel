# 传感器封装设计文档

## 架构概览

```
┌─────────────────────────────────────────────────────────────┐
│                      应用层 (Application)                    │
│         直接使用传感器类进行数据采集                          │
└─────────────────────────────────────────────────────────────┘
                              │
┌─────────────────────────────────────────────────────────────┐
│                      传感器层 (Sensor)                       │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │   Mpu6050    │  │    Dht11     │  │  其他传感器   │      │
│  │  (I2C接口)   │  │  (GPIO接口)  │  │              │      │
│  │              │  │              │  │              │      │
│  │ - init()     │  │ - init()     │  │              │      │
│  │ - read()     │  │ - read()     │  │              │      │
│  │ - readAccel()│  │ - getTemp()  │  │              │      │
│  │ - readGyro() │  │ - getHumid() │  │              │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
└─────────────────────────────────────────────────────────────┘
                              │
┌─────────────────────────────────────────────────────────────┐
│                    HAL层 (硬件抽象层)                        │
│  ┌──────────────┐  ┌──────────────┐                        │
│  │  DeviceNode  │  │   I2cBus     │                        │
│  │ (设备节点)   │  │  (I2C总线)   │                        │
│  │              │  │              │                        │
│  │ - open()     │  │ - open()     │                        │
│  │ - read()     │  │ - setSlave() │                        │
│  │ - write()    │  │ - readByte() │                        │
│  │ - ioctl()    │  │ - writeByte()│                        │
│  └──────────────┘  └──────────────┘                        │
└─────────────────────────────────────────────────────────────┘
                              │
┌─────────────────────────────────────────────────────────────┐
│                    Linux内核驱动层                           │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐   │
│  │  sysfs   │  │   I2C    │  │   SPI    │  │   GPIO   │   │
│  │ 接口     │  │  接口    │  │  接口    │  │  接口    │   │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘   │
└─────────────────────────────────────────────────────────────┘
```

## 目录结构

```
include/
└── hal/
    ├── DeviceNode.h      # 设备节点封装 (sysfs, /dev/*)
    └── I2cBus.h          # I2C总线封装
└── sensor/
    ├── Mpu6050.h         # MPU6050 六轴传感器
    └── Dht11.h           # DHT11 温湿度传感器

src/
└── hal/
    ├── DeviceNode.cpp
    └── I2cBus.cpp
└── sensor/
    ├── Mpu6050.cpp
    └── Dht11.cpp

examples/
└── sensor_example.cpp    # 使用示例
```

## HAL层设计

### DeviceNode - 设备节点封装

封装Linux设备文件的基础操作，适用于：
- sysfs接口（温度、GPIO、ADC等）
- /dev/设备节点

```cpp
// 使用示例
hal::DeviceNode dev;
dev.open("/sys/class/thermal/thermal_zone0/temp", O_RDONLY);
std::string value = dev.readString();
dev.close();
```

### I2cBus - I2C总线封装

封装I2C通信操作，适用于I2C传感器：
- MPU6050、BMP280、ADS1115等

```cpp
// 使用示例
hal::I2cBus i2c(0);  // I2C总线0
i2c.open();
i2c.setSlaveAddr(0x68);
int data = i2c.readByte(0x75);  // 读寄存器
i2c.close();
```

## 传感器层设计

### Mpu6050 - 六轴传感器

I2C接口的加速度计+陀螺仪传感器。

```cpp
sensor::Mpu6050 mpu;
sensor::Mpu6050Config config;
config.i2cBus = 0;
config.addr = 0x68;
config.accelRange = 2;  // ±8g
config.gyroRange = 1;   // ±500°/s

mpu.init(config);

sensor::Mpu6050Data data;
mpu.read(data);
// data.accelX, data.gyroX, data.temp...

mpu.deinit();
```

### Dht11 - 温湿度传感器

GPIO接口的单总线温湿度传感器。

```cpp
sensor::Dht11 dht(17, sensor::DHT11);  // GPIO17
dht.init();

sensor::DhtData data;
dht.read(data);
// data.temperature, data.humidity

float temp = dht.getTemperature();
float humid = dht.getHumidity();

dht.deinit();
```

## 常见设备路径

| 传感器类型 | 典型路径 |
|-----------|---------|
| CPU温度 | `/sys/class/thermal/thermal_zone0/temp` |
| hwmon温度 | `/sys/class/hwmon/hwmon0/temp1_input` |
| GPIO | `/sys/class/gpio/gpioN/value` |
| I2C | `/dev/i2c-N` |
| SPI | `/dev/spidevN.M` |
| ADC | `/sys/bus/iio/devices/iio:device0/in_voltage0_raw` |

## 扩展新传感器

1. 在HAL层添加需要的接口（如SpiBus）
2. 在Sensor层创建新的传感器类
3. 使用HAL层接口进行通信

```cpp
// 示例：BMP280气压传感器
class Bmp280 {
    hal::I2cBus i2c_;
public:
    bool init(int bus, uint8_t addr);
    bool read(Bmp280Data& data);
};
```

## 编译

源文件已通过 `aux_source_directory` 自动添加到编译：

```cmake
aux_source_directory(${PROJECT_SOURCE_DIR}/src/hal SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/sensor SRCS)
```

## 线程安全

- HAL层类使用RAII，析构时自动关闭
- 传感器类内部管理资源生命周期
- 支持多实例并发操作不同设备