# 命名空间防止冲突演示

## ❌ 没有命名空间 - 冲突场景

```cpp
// 文件1: vendor_lib/Mpu6050.h (第三方库)
class Mpu6050 {  // 全局命名空间
public:
    void readI2C();  // 直接I2C读取
};

// 文件2: my_project/Mpu6050.h (你的代码)
class Mpu6050 {  // 全局命名空间 - 冲突！
public:
    void readDeviceNode();  // 字符设备读取
};

// 使用时
#include "vendor_lib/Mpu6050.h"
#include "my_project/Mpu6050.h"  // ❌ 编译错误：重复定义 Mpu6050

Mpu6050 mpu;  // ❌ 编译器不知道用哪个 Mpu6050
```

**编译错误：**
```
error: redefinition of 'class Mpu6050'
note: previous definition of 'class Mpu6050' was here
```

---

## ✅ 有命名空间 - 完美隔离

```cpp
// 文件1: vendor_lib/Mpu6050.h (第三方库)
namespace vendor_lib {
    class Mpu6050 {  // vendor_lib::Mpu6050
    public:
        void readI2C();
    };
}

// 文件2: my_project/Mpu6050.h (你的代码)
namespace sensor {
    class Mpu6050 {  // sensor::Mpu6050 - 不冲突！
    public:
        void readDeviceNode();
    };
}

// 使用时
#include "vendor_lib/Mpu6050.h"
#include "sensor/Mpu6050.h"  // ✅ 编译成功

vendor_lib::Mpu6050 vendorMpu;  // 第三方库的版本
sensor::Mpu6050 myMpu;          // 你的版本
```

---

## 命名空间的5种使用方法

### 方法1: 完全限定名（最清晰）

```cpp
#include "sensor/Mpu6050.h"
#include "hal/DeviceNode.h"

int main() {
    sensor::Mpu6050 mpu;       // 明确指定 sensor 命名空间
    hal::DeviceNode dev;       // 明确指定 hal 命名空间
    
    sensor::Mpu6050Data data;  // 数据结构也在命名空间中
    mpu.read(data);
    
    return 0;
}
```

**优点：**
- ✅ 最清晰，一眼看出属于哪个命名空间
- ✅ 不会混淆
- ✅ 适合大型项目

---

### 方法2: using 声明（引入单个符号）

```cpp
#include "sensor/Mpu6050.h"

// 只引入 Mpu6050 类，不引入整个命名空间
using sensor::Mpu6050;
using sensor::Mpu6050Data;

int main() {
    Mpu6050 mpu;       // 不需要写 sensor:: 了
    Mpu6050Data data;  // 同样不需要
    
    mpu.read(data);
    return 0;
}
```

**优点：**
- ✅ 减少代码长度
- ✅ 只引入需要的符号
- ⚠️ 注意：不要在头文件中使用（会污染别人的代码）

---

### 方法3: using 命名空间（引入整个命名空间）

```cpp
#include "sensor/Mpu6050.h"

// 引入整个 sensor 命名空间
using namespace sensor;

int main() {
    Mpu6050 mpu;       // 直接使用
    Mpu6050Data data;  // 直接使用
    Dht11 dht;         // 直接使用
    
    mpu.read(data);
    return 0;
}
```

**优点：**
- ✅ 最简洁
- ⚠️ 缺点：可能引入不需要的符号
- ⚠️ 注意：**永远不要在头文件中使用**（会污染全局命名空间）

---

### 方法4: 嵌套命名空间（分层组织）

```cpp
// 嵌套命名空间定义
namespace company {
    namespace project {
        namespace sensor {
            class Mpu6050 { };
        }
    }
}

// C++17 简化写法
namespace company::project::sensor {
    class Mpu6050 { };
}

// 使用
company::project::sensor::Mpu6050 mpu;

// 或使用别名简化
namespace cps = company::project::sensor;
cps::Mpu6050 mpu;  // 简洁！
```

---

### 方法5: 匿名命名空间（文件内私有）

```cpp
// 文件: Mpu6050.cpp

namespace sensor {
    
    // 匿名命名空间 - 只在本文件可见
    namespace {
        const int BUFFER_SIZE = 14;  // 私有常量
        int internalCounter = 0;     // 私有变量
        
        void helperFunction() {      // 私有函数
            // ...
        }
    }
    
    class Mpu6050 {
        void read() {
            helperFunction();  // 可以访问匿名命名空间
            internalCounter++;
        }
    };
}

// 其他文件无法访问 BUFFER_SIZE, internalCounter, helperFunction
```

**用途：**
- ✅ 文件内的私有变量/函数
- ✅ 替代 `static` 关键字
- ✅ 防止符号导出

---

## 实际项目中的最佳实践

### ✅ 推荐做法

```cpp
// ===== 头文件 (Mpu6050.h) =====
namespace sensor {
    class Mpu6050 {  // 定义在命名空间中
        // ...
    };
}

// ===== 实现文件 (Mpu6050.cpp) =====
#include "sensor/Mpu6050.h"

namespace sensor {  // 在命名空间中实现
    Mpu6050::Mpu6050() {
        // ...
    }
}

// ===== 使用文件 (main.cpp) =====
#include "sensor/Mpu6050.h"

// ❌ 不要在头文件中使用 using namespace
// ✅ 在实现文件中使用完全限定名
int main() {
    sensor::Mpu6050 mpu;  // 清晰明确
    return 0;
}
```

---

## 常见错误与陷阱

### ❌ 错误1: 头文件中使用 using namespace

```cpp
// ❌ 错误示例: Mpu6050.h
#ifndef SENSOR_MPU6050_H
#define SENSOR_MPU6050_H

using namespace std;  // ❌ 汸染所有包含此头文件的代码！

namespace sensor {
    class Mpu6050 {
        string name;  // 不需要写 std::string
    };
}
#endif
```

**后果：**
- 所有包含此头文件的代码都被强制引入 `std` 命名空间
- 可能导致其他代码的命名冲突

**正确做法：**
```cpp
// ✅ 正确示例: Mpu6050.h
#ifndef SENSOR_MPU6050_H
#define SENSOR_MPU6050_H

#include <string>  // 只包含需要的头文件

namespace sensor {
    class Mpu6050 {
        std::string name;  // 使用完全限定名
    };
}
#endif
```

---

### ❌ 错误2: 跨命名空间依赖混乱

```cpp
// ❌ 错误示例
namespace hal {
    class DeviceNode;
}

namespace sensor {
    // ❌ 直接使用其他命名空间的类，没有明确声明
    class Mpu6050 {
        DeviceNode dev_;  // ❌ 编译器找不到 DeviceNode
    };
}

// ✅ 正确示例
namespace sensor {
    class Mpu6050 {
        hal::DeviceNode dev_;  // ✅ 明确指定命名空间
    };
}
```

---

## 你的项目实际使用

### 当前项目结构

```
rk3506-smart-panel/
├── include/
│   ├── hal/
│   │   └── DeviceNode.h      → namespace hal
│   └── sensor/
│       ├── Mpu6050.h         → namespace sensor
│       └── Dht11.h           → namespace sensor
└── src/
    ├── hal/
    │   └── DeviceNode.cpp    → namespace hal
    └── sensor/
        ├── Mpu6050.cpp       → namespace sensor
        └── Dht11.cpp         → namespace sensor
```

### 实际代码使用

```cpp
// ===== Mpu6050.h =====
namespace sensor {
    class Mpu6050 {
    private:
        hal::DeviceNode dev_;  // ✅ 跨命名空间使用，明确指定
    };
}

// ===== Mpu6050.cpp =====
namespace sensor {
    Mpu6050::Mpu6050(const std::string& devicePath)  // ✅ 在命名空间中实现
        : devicePath_(devicePath)
    {
    }
}

// ===== main.cpp =====
#include "sensor/Mpu6050.h"
#include "sensor/Dht11.h"

int main() {
    sensor::Mpu6050 mpu;  // ✅ 完全限定名
    sensor::Dht11 dht;    // ✅ 完全限定名
    
    sensor::Mpu6050Data mpuData;
    sensor::DhtData dhtData;
    
    return 0;
}
```

---

## 总结

| 使用场景 | 推荐方法 | 示例 |
|---------|---------|------|
| **头文件定义** | 定义在命名空间中 | `namespace sensor { class Mpu6050; }` |
| **头文件使用** | 完全限定名 | `hal::DeviceNode dev_;` |
| **实现文件** | 在命名空间中实现 | `namespace sensor { Mpu6050::read() { } }` |
| **使用文件** | 完全限定名 | `sensor::Mpu6050 mpu;` |
| **频繁使用** | using 声明 | `using sensor::Mpu6050;` |
| **私有符号** | 匿名命名空间 | `namespace { int privateVar; }` |

**核心原则：**
1. ✅ 所有代码都放在命名空间中
2. ✅ 头文件中永远不要使用 `using namespace`
3. ✅ 跨命名空间使用时明确指定
4. ✅ 使用完全限定名最清晰