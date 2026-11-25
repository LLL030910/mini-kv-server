## 所有权

在 C++ 中，**所有权指的是管理某个资源生命周期的责任，尤其是决定这个资源什么时候被释放。**

例如：

```c++
int* p = new int(100);
```

这行代码在堆上分配了一块内存。

使用结束后，需要执行：

```c++
delete p;
```

否则可能发生内存泄漏。这里可以认为，`p` 被约定为这块内存的所有者，因为程序需要通过它来管理和释放资源

```c++
auto p1 = std::make_shared<int>(100);
int* raw = p1.get();
```

- `p1` 拥有资源的共享所有权。
- `raw` 只是一个普通指针，可以访问对象。
- `raw` **不拥有资源**，不能延长对象生命周期。

## unique_ptr: 独占所有权

- 禁止拷贝构造和拷贝赋值
- 只能通过 `move` 转移所有权
- `move` 后原指针为 `nullptr`

### get() 借用指针

```c++
auto p = std::make_unique<int>(100);
int* raw = p.get();
std::cout << *raw << '\n';  // 100
```

- `p` 仍然管理 `int(100)`。
- `raw` 只是借用访问权。
- 不应该手动 `delete raw`，否则容易导致重复释放。

### release()：放弃所有权 

```c++
auto p = std::make_unique<int>(100);
int* raw = p.release();
std::cout << (p == nullptr) << '\n'; // 1
std::cout << *raw << '\n';           // 100

delete raw;
```

- `p` 变成 `nullptr`。
- 堆上的 `int(100)` 依然存在。
- 原始指针 `raw` 需要由调用方正确管理。

如果不写 `delete raw`，也没有把 `raw` 交给其他 RAII 管理对象，那么就会造成内存泄漏。

### reset()：销毁当前对象

`reset()` 会调用默认删除器，销毁原来管理的对象

### 题目1

```c++
auto p1 = std::make_unique<int>(10);
int* raw = p1.release();
auto p2 = std::unique_ptr<int>(raw);
std::cout << *p2 << '\n';
```

- 是否有内存泄漏？
- `raw` 接替 `p1`管理内存空间，`p2` 构造时指向 `raw` 指向的地址，最后 `p2` 自动调用析构 `delete` 释放，不需要 `raw` 手动释放，否则造成重复释放。

## shared_ptr : 共享所有权

和 `unique_ptr` 不同，`shared_ptr` 允许多个智能指针共同拥有同一个对象。

### 问题2

```c++
auto p1 = std::make_shared<int>(100);
std::shared_ptr<int> p2(p1.get());
```

- 这段代码有问题吗？
- 第二句中 `get()` 只是返回原始指针，不会转移所有权。**用原始指针构造新的** `shared_ptr`**，会让它认为自己获得了该指针的所有权，并创建独立的控制块。** 因此，`p2` 并不知道 `p1` 已经在管理这个对象。会造成多重释放

<img src="/home/lgj/study/projects/mini-kv-server/docs/resource/shared_ptr1.svg" alt="img" style="zoom:33%;" />

当程序结束时：

1. `p2` 析构，它自己的引用计数从 1 变成 0，尝试释放对象。
2. `p1` 随后析构，它自己的引用计数也从 1 变成 0，再次尝试释放同一个对象。

正确写法：

```c++
auto p1 = std::make_shared<int>(100);
auto p2 = p1;  // 正确：共享所有权
```

<img src="/home/lgj/study/projects/mini-kv-server/docs/resource/shared_ptr2.svg" alt="img" style="zoom:33%;" />

### 题目3

```c++
auto p1 = std::make_shared<int>(100);  // 引用计数 1
auto p2 = p1;                          // 引用计数 2
auto p3 = std::move(p2);               // 引用计数仍为 2
```

`std::move(p2) ` : `p2` **将自己的共享所有权转移给了** `p3`**，而不是释放了它所管理的资源。**

因为没有增加或减少共享所有者的总数，所以强引用计数保持为 2。

- 拷贝：共享所有者数量 +1。
- 移动：共享所有者数量不变（这里 `p3` 是新创建的空目标）。
- `reset()`：当前指针放弃所有权，共享所有者数量通常 -1。

## weak_ptr

`weak_ptr` 它指向一个由 `shared_ptr` 管理的对象，但**不增加引用计数**，因此不会阻止对象被销毁。它的核心用途是**打破循环引用**和**安全地探测对象是否还活着**。

### 基本构造

`weak_ptr` 只能从 `shared_ptr` 或另一个 `weak_ptr` 构造，**不能**从裸指针构造：

```c++
auto sp = std::make_shared<int>(42);

std::weak_ptr<int> wp(sp);        // 从 shared_ptr 构造
std::weak_ptr<int> wp2 = sp;      // 同上
std::weak_ptr<int> wp3 = wp;      // 从另一个 weak_ptr 构造
// std::weak_ptr<int> bad(ptr);   // ❌ 不允许从裸指针构造
```

构造 `weak_ptr` **不改变** `sp` 的引用计数（强引用计数不变，弱引用计数 +1）

### 探测与获取对象：`lock()` 和 `expired()`

#### lock()

`weak_ptr` 不保证对象仍然存活，因此必须通过 `lock()` 检查对象是否存在，并在访问期间临时持有它的共享所有权，访问前必须**先"提升"为 `shared_ptr`**。

```c++
if (auto sp = wp.lock()) {        // 若对象存活，返回有效 shared_ptr（计数+1）
    // 安全使用 *sp
} else {
    // 对象已销毁
}
```

#### expired() - 仅探测，不获取

```c++
if (wp.expired()) {
    // 对象已销毁（强引用计数为 0）
}
```

只告诉你对象是否已死，**不**给你访问权。注意：即使 `expired()` 返回 `false`，也不能保证下一刻对象仍然存活。

### `use_count()` —— 观察强引用计数

### 问题1：循环引用

```c++
#include <iostream>
#include <memory>

class B;

class A {
public:
    std::shared_ptr<B> b;
    ~A() {
        std::cout << "A destroyed\n";
    }
};

class B {
public:
    std::shared_ptr<A> a;
    ~B() {
        std::cout << "B destroyed\n";
    }
};

int main() {
    auto p1 = std::make_shared<A>();
    auto p2 = std::make_shared<B>();
    p1->b = p2;
    p2->a = p1;
    std::cout << p1.use_count() << '\n'; // 2
    std::cout << p2.use_count() << '\n'; // 2
}
```

程序正常结束时，`A` **和** `B` **的析构函数都不会执行**，造成内存泄漏。

原因是：

- `p1` 释放所有权后，`B` 内部的 `shared_ptr<A>` 仍然拥有 `A`。
- `p2` 释放所有权后，`A` 内部的 `shared_ptr<B>` 仍然拥有 `B`。
- 两个对象互相保持对方存活，谁也无法正常销毁。

这种现象称为**循环引用（Circular Reference）**。解决方法是将其中一个成员改成 `weak_ptr`