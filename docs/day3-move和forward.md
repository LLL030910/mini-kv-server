## 1. 左值、右值

**左值**通常代表可以持续访问、具有身份的对象；**右值**通常包括临时计算结果，以及明确允许其资源被转移的表达式。

```c++
int a = 10;

// 左值引用
int& ref1 = a;

// 右值引用
int&& ref2 = 20;

int&& ref3 = ref2; // 编译错误
```

因为 `ref2` **虽然声明为右值引用，但是作为一个有名字的变量表达式，它是左值。**

因此：

```c++
int&& ref3 = std::move(ref2); // 正确
```

## 2. `const` 左值引用可以绑定右值

```c++
const int& r6 = 20; // 实际上是正确的
```

C++ 允许通过 `const` 左值引用绑定这个临时值，并延长相应临时对象的生命周期。
这里临时对象的生命周期会延长至引用变量 `r` 的生命周期结束。
但由于引用具有 `const` 限定，不能通过 `r` 修改这个值：

```c++
r = 30; // 编译错误
```

这种机制使函数能够高效地接收临时对象，而不必复制它们。

```c++
void print(const std::string& str) {
    std::cout << str << '\n';
}

print(std::string("hello")); // 正确
```

## 3. `move`

### 3.1 问题1

```c++
#include <iostream>
#include <utility>

class Test {
public:
    Test() {
        std::cout << "Default constructor\n";
    }
    Test(const Test& other) {
        std::cout << "Copy constructor\n";
    }
    Test(Test&& other) noexcept {
        std::cout << "Move constructor\n";
    }
};

int main() {
    Test t1;               // default
    Test t2(t1);           // copy
    Test t3(std::move(t1)); // move
    Test&& ref = Test();    // default，然后并不会调用move
    Test t4(ref);           // copy
    Test t5(std::move(ref)); // move
}
```

`Test&& ref = Test();` 执行过程是：

1. `Test()` 创建一个临时对象，调用默认构造函数。
2. `ref` 是右值引用，直接绑定到这个临时对象。
3. 没有创建第二个 `Test` 对象，因此也就不需要移动构造。

包括 `Test& ref = Test();` 也是一样的流程，不会调用拷贝构造。

### 3.2 问题2

`t2` 和 `t3` 分别调用什么构造函数？

```c++
Test t1;
Test& ref1 = t1;
Test t2 = ref1;     // copy
Test&& ref2 = std::move(t1);
Test t3 = ref2;     // copy
```

`ref2` **的类型是** `Test&&`**（右值引用类型），但表达式** `ref2` **是左值（lvalue）。**

| 表达式            | 声明类型 | 表达式的值类别              |
| ----------------- | -------- | --------------------------- |
| `t1`              | `Test`   | 左值                        |
| `ref1`            | `Test&`  | 左值                        |
| `ref2`            | `Test&&` | 左值                        |
| `std::move(t1)`   | `Test&&` | 将结果表达式归类为 `xvalue` |
| `std::move(ref2)` | `Test&&` | 将结果表达式归类为 `xvalue` |

`std::move(x)` 返回的结果是右值，但不影响 `x` 的类型。

## 4. `noexcept` **与** `std::vector` **扩容**

### 4.1 `explicit`关键字

1. `explicit` 限制的到底是哪种类型转换？
2. `emplace_back()` 和 `push_back()` 有什么区别？
3. `emplace_back()` 为什么能直接传递构造函数参数？

```c++
class Tracer {
public:
    explicit Tracer(std::string name)
        : name_(std::move(name)) {
        std::cout << "Constructor: " << name_ << '\n';
    }
private:
    std::string name_;
};
```

`explicit` 的主要作用是**不允许隐式地将 `std::string` 转换为 `Tracer`**，但允许直接初始化。

```c++
Tracer t1("A");                   // 正确
Tracer t = "A";          	      // ❌ 错误：隐式转换被 explicit 禁止
void f(Tracer); f("A");           // ❌ 错误：隐式转换被 explicit 禁止
```

`emplace_back` 的语义是：**把参数完美转发到容器内存里，直接在目标位置调用 `Tracer` 的构造函数**。

| 写法                            | 是否正确 | 行为                               |
| ------------------------------- | -------- | ---------------------------------- |
| `vec.emplace_back("A")`         | ✅ 推荐   | 原位构造 `Tracer`                  |
| `vec.emplace_back(Tracer("A"))` | ✅        | 先创建临时对象，再移动进容器       |
| `vec.push_back(Tracer("A"))`    | ✅        | 创建临时对象，再移动进容器         |
| `vec.push_back("A")`            | ❌        | `explicit` 阻止隐式转换成 `Tracer` |

`emplace_back` 的实际实现利用了完美转发，将参数传递给元素的构造函数。

### 4.2 问题1

**实验 A：移动构造带** `noexcept`

```c++
Tracer(Tracer&& other) noexcept
    : name_(std::move(other.name_)) {
    std::cout << "Move: " << name_ << '\n';
}
```

**实验 B：删除** `noexcept`

```c++
Tracer(Tracer&& other)
    : name_(std::move(other.name_)) {
    std::cout << "Move: " << name_ << '\n';
}
```

两个实验都使用：

```c++
int main() {
    std::vector<Tracer> vec;

    vec.reserve(1);
    vec.emplace_back("A");

    std::cout << "---- Add B ----\n";

    vec.emplace_back("B");
}
```

**输出结果：**

```c++
Constructor: A
---- Add B ----
Constructor: B
Move: A    // 带 noexcept 移动构造， 扩容时将 A 移动到新存储空间。
Copy: A    // 不带noexcept 拷贝构造，扩容时复制 A，以便发生异常时尽可能保留原有元素。
```

假设 `vector` 原来保存了一个对象 A，现在需要扩容。如果移动构造可能抛出异常，那么移动 A 的过程中，原来的 A 可能已经被修改。

假如此时发生异常，`vector` 就很难保证原来的数据完全不变。而拷贝通常不会修改原对象。因此，只要拷贝成功之前不销毁原对象，就更容易提供**强异常安全保证**：操作失败时，容器原有状态保持不变。

## 5. `forward`

### 5.1 问题1

```c++
#include <iostream>
#include <string>
#include <utility>

void process(const std::string& str) {
    std::cout << "Lvalue version\n";
}
void process(std::string&& str) {
    std::cout << "Rvalue version\n";
}
// 1.模板参数 T&& 在满足类型推导条件时是转发引用，可以接收左值和右值
template<typename T>
void wrapper(T&& value) {
    // 2.但进入函数后，具有名字的 value 表达式始终是左值。这里始终调用 Lvalue
    process(value);
}

int main() {
    std::string name = "Linux";
    wrapper(name);
    wrapper(std::string("Server"));
}
```

### 5.2 完美转发

```c++
template<typename T>
void wrapper(T&& value) {
    process(std::forward<T>(value));
}
```

这里的 `T&&` 是**转发引用（Forwarding Reference）**。

模板推导结果分别是：

| 调用                             | T 推导结果     | `std::forward<T>(value)` |
| -------------------------------- | -------------- | ------------------------ |
| `wrapper(name)`                  | `std::string&` | 左值                     |
| `wrapper(std::string("Server"))` | `std::string`  | 右值（`xvalue`）         |

`std::forward` 不是根据运行时 `value` 保存的对象来判断左值还是右值，而是**根据编译阶段推导出来的模板参数 T 保留原始实参的值类别**。