# DeviceNode C++17 现代化改进

## 概览

将 `DeviceNode` 类从传统C++风格重构为符合C++17标准的现代设计，提升代码的类型安全性、错误处理能力和性能。

## 关键改进

### 1. **错误处理：`std::optional<std::error_code>`**

**旧方式（布尔返回值）：**
```cpp
// 无法获知失败原因
bool success = device.open("/dev/i2c-0");
if (!success) {
    // 为什么失败？权限问题？文件不存在？
    std::cerr << "Open failed\n";
}
```

**新方式（C++17 `std::optional`）：**
```cpp
// 明确的错误信息
auto error = device.open("/dev/i2c-0");
if (error) {  // std::nullopt 表示成功，error_code 表示失败
    std::cerr << "Failed: " << error->message() << "\n";
} else {
    // 成功
}
```

### 2. **参数优化：`std::string_view`**

**旧方式：**
```cpp
bool open(const std::string& path, int flags = O_RDWR);
// 需要创建临时 std::string 对象
device.open("/dev/i2c-0");  // 隐式转换
```

**新方式（C++17）：**
```cpp
std::optional<std::error_code> 
open(std::string_view path, int flags = O_RDWR) noexcept;
// 零拷贝，支持 C 风格字符串、std::string、字符串字面量
device.open("/dev/i2c-0");
const char* path = "/dev/i2c-0";
device.open(path);
std::string str = "/dev/i2c-0";
device.open(str);
```

### 3. **返回值属性：`[[nodiscard]]`**

**意图：** 强制开发者检查函数返回值

```cpp
[[nodiscard]] std::optional<std::error_code> 
open(std::string_view path, int flags = O_RDWR) noexcept;

// 编译器警告：返回值被忽略！
device.open("/dev/i2c-0");  // ⚠️ Warning

// 正确做法
auto error = device.open("/dev/i2c-0");
if (error) { /* 处理错误 */ }
```

### 4. **异常安全：`noexcept` 规范**

```cpp
DeviceNode() noexcept;
~DeviceNode() noexcept;
bool isOpen() const noexcept;
void close() noexcept;

// 告诉编译器/调用者这些函数不会抛异常
// 有利于优化和容器的高效操作
```

### 5. **Move 语义支持**

**新增：**
```cpp
DeviceNode(DeviceNode&& other) noexcept;
DeviceNode& operator=(DeviceNode&& other) noexcept;

// 现在支持 move，可以放入容器
std::vector<DeviceNode> devices;
DeviceNode dev;
dev.open("/dev/i2c-0");
devices.push_back(std::move(dev));  // 高效地转移所有权
```

### 6. **可选返回值**

**旧方式（返回空字符串表示失败）：**
```cpp
std::string str = device.readString();
if (str.empty()) {
    // 失败还是读到空字符串？不知道！
}
```

**新方式（C++17 `std::optional`）：**
```cpp
auto result = device.readString();
if (result.has_value()) {
    std::cout << result.value() << "\n";
} else {
    std::cout << "Read failed\n";
}

// 或使用 C++17 结构化绑定
if (auto str = device.readString()) {
    std::cout << *str << "\n";
}
```

## 实际使用示例

### 使用模式对比

**旧版本：**
```cpp
DeviceNode device;
if (!device.open("/sys/class/hwmon/hwmon0/temp1_input")) {
    std::cerr << "Open failed\n";
    return -1;
}

std::string temp = device.readString();
if (temp.empty()) {
    std::cerr << "Read failed\n";
    return -1;
}

if (!device.writeString("100")) {
    std::cerr << "Write failed\n";
    return -1;
}
device.close();
```

**新版本（更清晰的错误处理）：**
```cpp
DeviceNode device;

if (auto err = device.open("/sys/class/hwmon/hwmon0/temp1_input")) {
    std::cerr << "Open failed: " << err->message() << "\n";
    return -1;
}

auto temp = device.readString();
if (!temp) {
    std::cerr << "Read failed\n";
    return -1;
}
std::cout << "Temperature: " << *temp << "\n";

if (auto err = device.writeString("100")) {
    std::cerr << "Write failed: " << err->message() << "\n";
    return -1;
}
// 不需要显式 close()，RAII 自动清理
```

### 容器使用

```cpp
// 现在可以安全地存储 DeviceNode
std::vector<DeviceNode> sensors;

DeviceNode sensor1, sensor2;
if (auto err = sensor1.open("/dev/sensor0"); !err) {
    sensors.push_back(std::move(sensor1));
}
if (auto err = sensor2.open("/dev/sensor1"); !err) {
    sensors.push_back(std::move(sensor2));
}

// Move 语义确保高效操作，析构函数正确清理资源
```

## C++17 标准库特性速查

| 特性 | 用途 | 示例 |
|------|------|------|
| `std::optional<T>` | 表示"有值或无值" | `std::optional<int>` |
| `std::string_view` | 只读字符串视图，零拷贝 | `std::string_view path` |
| `[[nodiscard]]` | 编译器强制检查返回值 | `[[nodiscard]] int func()` |
| `noexcept` | 声明函数不抛异常 | `void func() noexcept` |
| `std::error_code` | 系统错误信息 | `std::error_code(errno, std::system_category())` |

## 编译要求

- **C++ 标准：** C++17 或更高
- **编译器标志：** `-std=c++17` (GCC/Clang) 或 `/std:c++17` (MSVC)
- **CMakeLists.txt 配置：**
  ```cmake
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  ```

## 向后兼容性说明

现有代码使用此库需要调整：

```cpp
// 旧代码需要修改：
device.open("/dev/i2c-0");  // 错误：需要检查返回值 ([[nodiscard]])

// 修改为：
if (auto err = device.open("/dev/i2c-0")) {
    // 处理错误
}
```

## 性能优势

1. **零拷贝参数传递** - `std::string_view` 避免不必要的字符串拷贝
2. **Move 语义** - 高效的资源转移，特别是在容器操作中
3. **编译期优化** - `noexcept` 帮助编译器生成更优的代码
4. **无额外开销** - `std::optional` 零成本抽象

## 总结

这次现代化重构：
- ✅ 提升类型安全性（`std::optional` 替代 bool）
- ✅ 改进错误处理（获取具体错误信息）
- ✅ 减少拷贝开销（`std::string_view`）
- ✅ 支持 Move 语义
- ✅ 强制错误检查（`[[nodiscard]]`）
- ✅ 更清晰的意图声明（`noexcept`）
