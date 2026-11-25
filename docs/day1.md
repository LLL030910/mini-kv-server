# 移动构造和移动赋值

## 1. explicit关键字

防止隐式转换风险**不允许隐式地将 `fd` 转换为 `UniqueFd`**

```cpp
UniqueFd(int fd) noexcept;   // 非 explicit 的单参数构造函数
```

这是非 explicit 的转换构造函数（converting constructor），编译器会允许 `int` 隐式转换为 `UniqueFd`。

```cpp
UniqueFD(5)；这里5有隐式转换的风险，可能直接转换成对象
```

## 移动构造

- 参数是**右值引用** `T&&`（非 const，因为要修改源对象）

```c++
class Buffer {
    char* data_;
    size_t size_;
public:
    Buffer(Buffer&& other) noexcept
        : data_(other.data_),      // 1. 接管资源
          size_(other.size_) {
        other.data_ = nullptr;      // 2. 源对象置空
        other.size_ = 0;            //  防止 double free
    }
};
```

**移动构造：** 当前对象尚未持有资源，所以直接接管资源即可，所以不需要判断是否自我赋值。

## 移动赋值

- 需要**自赋值检查**（`if (this != &other)`）
- 需要**先释放自身已有资源**（避免内存泄漏）
- 返回 `*this` 以支持链式赋值

```c++
Buffer& operator=(Buffer&& other) noexcept {
    if (this != &other) {          // 1. 自赋值保护
        delete[] data_;            // 2. 释放自身资源
        data_ = other.data_;       // 3. 接管资源
        size_ = other.size_;
        other.data_ = nullptr;     // 4. 源对象置空
        other.size_ = 0;
    }
    return *this;                  // 5. 返回引用
}
```

**移动赋值：** 当前对象可能已经持有资源，必须先处理旧资源，再接管新资源。

## std::move()

**`std::move` 本身不执行任何移动操作，也不调用移动构造/移动赋值**。它的唯一作用是把左值**强制转换**为右值引用（`T&&`），从而让后续的重载决议选中移动版本。

```c++
Buffer a;
Buffer b(std::move(a));   // ① std::move(a) 只是把 a 转成 Buffer&&
                          // ② 真正调用移动构造函数 Buffer(Buffer&&) 的是 ① 的结果
                          // ③ 移动构造函数内部才接管 a 的资源、把 a 置空

Buffer c;
c = std::move(b);         // ① std::move(b) 转成 Buffer&&
                          // ② 真正调用移动赋值 operator=(Buffer&&) 的是 ① 的结果
```

