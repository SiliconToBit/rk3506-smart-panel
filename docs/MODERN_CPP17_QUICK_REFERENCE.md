# DeviceNode 现代C++17改进 - 快速参考

## 迁移指南

### ✅ 改进清单

| 特性 | 旧API | 新API | 优势 |
|------|-------|-------|------|
| **错误处理** | `bool open()` | `std::optional<std::error_code>` | 获取具体错误信息 |
| **参数优化** | `const std::string&` | `std::string_view` | 零拷贝，统一接口 |
| **返回值** | `std::string readString()` | `std::optional<std::string>` | 区分空值和失败 |
| **异常安全** | 无声明 | `noexcept` | 编译器优化 |
| **Move支持** | 不可移动 | 支持Move构造/赋值 | 容器友好 |
| **意图标记** | 无 | `[[nodiscard]]` | 强制检查返回值 |

---

## 实际代码改写示例

### 示例1：基本操作

<details>
<summary><b>旧API</b></summary>

```cpp
DeviceNode dev;
if (!dev.open("/dev/i2c-0")) {
    cerr << "Fail to open\n";  // 原因不明
    return;
}

string val = dev.readString();
if (val.empty()) {
    cerr << "Read failed?\n";  // 不确定
}

dev.close();
```
</details>

<details>
<summary><b>新API</b></summary>

```cpp
DeviceNode dev;
if (auto err = dev.open("/dev/i2c-0")) {
    cerr << "Failed: " << err->message() << "\n";  // 明确的错误信息
    return;
}

if (auto val = dev.readString()) {
    cout << "Value: " << *val << "\n";  // 明确有值
} else {
    cerr << "Read failed\n";  // 明确失败
}
// 自动 RAII 关闭，无需显式 close()
```
</details>

---

### 示例2：字符串参数

<details>
<summary><b>旧版本（可能产生拷贝）</b></summary>

```cpp
// 方式1：C 字符串需要隐式转换
dev.open("/dev/i2c-0");  // 创建临时 std::string

// 方式2：std::string 可能产生拷贝
std::string path = "/dev/i2c-0";
dev.open(path);  // 拷贝构造

// 方式3：无法统一处理
```
</details>

<details>
<summary><b>新版本（零拷贝）</b></summary>

```cpp
// 所有方式都零拷贝，无论是什么源
dev.open("/dev/i2c-0");           // 字符串字面量
dev.open(std::string("..."));     // 临时对象
dev.open(my_string_variable);     // std::string 引用
const char* path = "...";
dev.open(path);                   // C 字符串

// 统一参数类型：std::string_view 支持所有这些
```
</details>

---

### 示例3：容器存储

<details>
<summary><b>旧版本（不可移动，有问题）</b></summary>

```cpp
vector<DeviceNode> devices;  // 编译基本通过

DeviceNode dev;
dev.open("/dev/sensor0");
devices.push_back(dev);  // ❌ 拷贝被禁用 -> 编译错误！

// 无法在容器中使用
```
</details>

<details>
<summary><b>新版本（Move 支持）</b></summary>

```cpp
vector<DeviceNode> devices;

DeviceNode dev;
dev.open("/dev/sensor0");
devices.push_back(std::move(dev));  // ✅ Move 构造，高效转移

// 或者直接嵌入初始化
devices.push_back(DeviceNode());
devices.back().open("/dev/sensor1");
```
</details>

---

## 错误码使用

```cpp
if (auto err = dev.open(path)) {
    // err 是 std::optional<std::error_code>，持有错误
    
    std::cout << err->value();      // 错误号（e.g., ENOENT）
    std::cout << err->message();    // 错误说明（e.g., "No such file or directory"）
    std::cout << err->category();   // 错误分类
    
    // 检查特定错误
    if (err->value() == ENOENT) {
        std::cout << "文件不存在\n";
    }
}
```

---

## C++17 标准库速参

### std::optional<T>
```cpp
std::optional<int> maybe_val;

if (maybe_val.has_value()) { ... }  // 检查是否有值
if (maybe_val) { ... }              // 等同上
int val = *maybe_val;               // 解引用
int val = maybe_val.value_or(0);    // 提供默认值
```

### std::string_view
```cpp
void func(std::string_view str) { ... }

func("hello");              // C 字符串，无拷贝
func(std::string("hi"));    // std::string，无拷贝
std::string_view sv = "...";
func(sv);                   // 字符串视图，无拷贝
```

### [[nodiscard]]
```cpp
[[nodiscard]] std::optional<int> risky_operation();

risky_operation();  // ⚠️ 编译警告：返回值被忽略

auto result = risky_operation();  // ✅ OK
std::ignore = risky_operation();  // ✅ 显式忽略
```

---

## 编译要求

```cmake
# CMakeLists.txt
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

```bash
# 编译器命令行
g++ -std=c++17 main.cpp
clang++ -std=c++17 main.cpp
```

---

## 常见问题

**Q: `std::optional` 有性能开销吗？**
A: 没有。它是零成本抽象，编译器会优化成与直接存储相同的性能。

**Q: 为什么使用 `std::string_view` 而不是 `const std::string&`？**
A: `string_view` 可以接受 C 字符串和字符串字面量，无需临时对象和拷贝。

**Q: Move 构造函数的目的是什么？**
A: 允许将对象放入容器，高效传递所有权，典型案例：`vector.push_back(std::move(dev))`

**Q: 必须升级现有代码吗？**
A: 建议升级。新 API 更安全、性能更好、错误处理更清晰。

---

## 参考

- [cppreference: std::optional](https://en.cppreference.com/w/cpp/utility/optional)
- [cppreference: std::string_view](https://en.cppreference.com/w/cpp/string/basic_string_view)
- [cppreference: [[nodiscard]]](https://en.cppreference.com/w/cpp/language/attributes/nodiscard)
- [C++17 标准](https://en.cppreference.com/w/cpp/17)

