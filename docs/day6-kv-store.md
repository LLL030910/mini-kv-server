## 1. TCP 字节流与粘包、拆包

假设客户端发送的是两条 KV 命令：

```shell
SET name Alice\n
GET name\n
```

服务器调用 `recv()` 时，能否保证第一次接收到 `SET` 命令，第二次接收到 `GET` 命令？

**不能。TCP 是面向字节流的协议，而不是面向消息的协议。**

![](/home/lgj/study/projects/mini-kv-server/docs/resource/6_tcp粘包.png)

TCP 保证字节流可靠、有序地传输，但不保证发送和接收时的消息边界一致。

因此：

- 一次 `send()` 不一定对应一次 `recv()`。
- 一次 `recv()` 可能获得多条消息。
- 一条消息可能需要多次 `recv()` 才能接收完整。
- `recv()` 返回的字节数不能直接用于判断一条业务消息是否接收完成。

## 2. Buffer 接收缓冲区

`char buf[4096]` 是临时读取缓冲区。

```c++
char buf[4096];

ssize_t n = recv(fd, buf, sizeof(buf), 0);
```

服务端由上到下处理数据

<img src="/home/lgj/study/projects/mini-kv-server/docs/resource/6_buffer.svg" style="zoom:33%;" />

| 缓冲区            | 所属位置     | 作用                   |
| ----------------- | ------------ | ---------------------- |
| Socket 接收缓冲区 | 操作系统内核 | 保存网络收到的数据     |
| `char buf[4096]`  | 用户空间     | 临时保存本次读取的数据 |
| `Buffer` 对象     | 用户空间     | 保存尚未处理的业务数据 |

## 3. `optional`

### 3.1 使用方法

`std::optional<T>` 表示一个可能存在、也可能不存在的 T 类型的值。

```c++
#include <optional>
#include <string>
#include <iostream>

std::optional<std::string> get_value(bool success) {
    if (success) {
        return "Hello";
    }
    return std::nullopt;
}

int main() {
    auto result = get_value(true);
    if (result.has_value()) {
        std::cout << result.value() << '\n';
    }
}
```

常见操作：

| 操作                 | 含义                       |
| -------------------- | -------------------------- |
| `std::nullopt`       | 不存在有效值               |
| `result.has_value()` | 判断是否存在值             |
| `if (result)`        | 判断是否存在值             |
| `result.value()`     | 获取内部值，无值时抛出异常 |
| `*result`            | 解引用，要求内部存在值     |

### 3.2 为什么消息解析需要 `optional`

考虑以下三种情况：

| Buffer 内容 | 完整消息     | 期望返回结果   |
| ----------- | ------------ | -------------- |
| `"Hello\n"` | Hello        | `"Hello"`      |
| `"Hello"`   | 尚未接收完整 | `std::nullopt` |
| `"\n"`      | 一条空消息   | `""`           |

如果直接返回 `std::string`，后两种情况就很难区分。

使用 `std::optional<std::string>` 可以明确区分：

```c++
std::optional<std::string> a = "";

std::optional<std::string> b = std::nullopt;
```

其中：

```c++
a.has_value();  // true
b.has_value();  // false
```

虽然 `a` 内部保存的是空字符串，但它仍然包含有效值。

## 4. kv_store

### 4.1 `unordered_map`

#### 1. `erase()`

1. **根据键（`Key`）删除 —— 返回删除的元素个数**

```c++
size_type erase(const Key& key);
```

- 作用：删除键为 `key` 的元素。

- 返回值：被删除的元素数量。因为是 `unordered_map`（键唯一），所以返回值要么是 1（删除成功），要么是 0（没找到该键）。

- 注意：如果不关心是否删除成功，可以直接调用。如果关心，可以用返回值判断。

2. **根据迭代器（Iterator）删除 —— 返回下一个迭代器**

- 作用：删除迭代器 `pos` 指向的单个元素。

- 返回值：指向被删除元素之后元素的迭代器（C++11 及以后）。

```c++
for (auto it = map.begin(); it != map.end(); ) {
    if (it->second > 10) {
        it = map.erase(it); // 用返回值更新迭代器，安全
    } else {
        ++it;
    }
}
```

3. **根据迭代器范围删除**

```c++
iterator erase(const_iterator first, const_iterator last);
```

- 作用：删除范围 `[first, last)` 内的所有元素。
- 返回值：指向 `last` 之后元素的迭代器（即返回 `last`）。
- 注意：`first` 和 `last` 必须是有效的迭代器范围，如果 `first == last`，则不删除任何元素。

### 4.2 插入

1. `insert()` —— 不覆盖已有值

`	insert()` 的特点是：如果键（key）已经存在，插入会失败，原值不会被覆盖。

```c++
std::unordered_map<std::string, int> map;

// 方法 1: 插入 pair
auto [it1, success1] = map.insert({"apple", 1}); // C++17 结构化绑定
if (success1) {
    std::cout << "Inserted: " << it1->first << " = " << it1->second << "\n";
}

// 方法 2: 插入 value_type
map.insert(std::make_pair("banana", 2));

// 尝试插入已存在的 key
auto [it2, success2] = map.insert({"apple", 100}); 
// success2 为 false，map["apple"] 依然是 1
```

2. `operator[]` —— 允许覆盖 / 插入

​	`operator[]` 的特点是：**如果键存在，返回对应值的引用；如果不存在，会先插入一个默认值（例如 int 为 0，string 为空），再返回引用。**

3. `insert_or_assign()` (C++17) —— 覆盖或插入

​	如果存在就覆盖，不存在就插入

### 4.3 store应该什么时候创建

| <img src="/home/lgj/study/projects/mini-kv-server/docs/resource/6_kv_store.svg" width="300" /> | <img src="/home/lgj/study/projects/mini-kv-server/docs/resource/6_kv_store2.svg" width="300" /> |

注意两个对象的生命周期：

| 对象                  | 生命周期               | 创建次数     |
| --------------------- | ---------------------- | ------------ |
| `KVStore`             | 整个服务器运行期间     | 一次         |
| `Buffer`              | 单个客户端连接期间     | 每个连接一次 |
| `UniqueFd listen_fd_` | TcpServer 对象存活期间 | 一次         |
| `UniqueFd client`     | 单个客户端连接期间     | 每个连接一次 |

**KVStore 是服务器共享的状态，Buffer 是连接私有的状态。**

## 5. `std::istringstream`

`std::cin` 从标准输入中提取数据，而 `std::istringstream` 可以从字符串中提取数据。

```c++
#include <sstream>
#include <iostream>
#include <string>

int main() {
    std::string input = "SET name Alice";

    std::istringstream iss(input);

    std::string cmd;
    std::string key;
    std::string value;

    iss >> cmd >> key >> value;

    std::cout << cmd << '\n';             // SET
    std::cout << key << '\n';             // name
    std::cout << value << '\n';           // Alice
}
```

### 5.1 如何判断参数不足？

例如客户端发送：`SET name` 此时没有 Value。

可以这样判断：

```c++
std::istringstream iss("SET name");

std::string cmd;
std::string key;
std::string value;

if (!(iss >> cmd >> key >> value)) {
    std::cout << "Invalid command\n";
}
```

当输入流无法提取所需的数据时，可以通过流的状态判断读取失败。

### 5.2 如何判断参数过多？

例如：`GET name age` , 虽然成功读取了 `GET` 和 `name`，但仍然存在多余参数 `age`。

可以这样检测：

```c++
std::string extra;

if (iss >> extra) {
    // 存在多余参数
}
```

