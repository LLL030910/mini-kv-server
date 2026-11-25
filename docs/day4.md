## 1. 文件描述符

在 Linux 中，进程访问打开的文件、Socket、管道等资源时，通常使用文件描述符（File Descriptor）。

文件描述符是一个非负整数，进程通过它找到对应的内核文件对象。

### `open()`

```c++
#include <fcntl.h>

int open(const char *pathname, int flags);
int open(const char *pathname, int flags, mode_t mode);  // 带 O_CREAT 时用，一般mode为0644
//  成功：返回最小可用的 fd（通常 ≥ 3，0/1/2 是 stdin/stdout/stderr）
//  失败：返回 -1，errno 给出原因
```

| 标志         | 含义                                                         |
| ------------ | ------------------------------------------------------------ |
| `O_CREAT`    | 文件不存在则创建（需要第三个 `mode` 参数）                   |
| `O_TRUNC`    | 文件存在且可写则截断为 0 长度（**生效时机：打开文件时**）    |
| `O_APPEND`   | 每次调用 `write`时，内核都会将写入位置定位到文件末尾，而不是仅在 `open`时定位一次。（原子，多进程安全）（**生效时机：每次写入时**） |
| `O_NONBLOCK` | 非阻塞模式                                                   |

### `read()`

```c++
ssize_t n = read(fd, buf, size);
// 返回 n > 0    成功读取 n 字节
// 返回 n == 0   对于普通文件，通常表示到达 EOF
// 返回 n == -1  发生错误，检查 errno
```

**EINTR** : 如果系统调用在尚未完成数据传输时被信号中断，可能出现：`n == -1 && errno == EINTR` ，这通常不意味着文件无法继续写入，而是应该重新尝试调用。

**EAGAIN**: **当前暂时没有数据可读，不代表连接失效。**

### 问题

1. 已经使用了 `O_APPEND`，在 `read_all()` 中调用 `lseek(fd, 0, SEEK_SET)` 之后，还能保证下一次 `write()` 写入文件末尾吗？
   - 保证。`open` 时使用 `O_APPEND`，每次调用 `write()` 时，内核都会自动将写入位置定位到文件末尾。即使当前文件偏移量为 0，也不会覆盖原有内容。

2.  O_APPEND 和 O_TRUNC 能否同时使用？

   - 可以同时使用，`O_TRUNC` 不是将文件偏移量移动到开头，而是将已有普通文件的长度截断为 0。

     

## 2. 文件偏移量

文件偏移量（File Offset）表示下一次读写操作在文件中的当前位置。`read()` 和 `write()` 成功传输数据后，普通文件的偏移量会随实际传输的字节数移动。

`lseek`

```c++
lseek(fd, 0, SEEK_CUR)	  // 获取当前偏移量
lseek(fd, 0, SEEK_SET);   // 表示相对于文件开头移动 0 字节，也就是回到文件开头。

lseek(fd, 2, SEEK_CUR)	  // 从当前位置向后移动 2 字节
lseek(fd, -2, SEEK_CUR)	  // 从当前位置向前移动 2 字节
lseek(fd, 5, SEEK_SET)	  // 将偏移量设置为 5
lseek(fd, 0, SEEK_END)	  // 移动至文件末尾
```

## 3. sizeof和strlen

### 问题1

假设有以下代码：

```c++
std::string str = "Hello";
const char* p = str.data();
char arr[] = "Hello";
```

在你的 64 位 Ubuntu 环境下，请判断：

| 表达式                              | 答案  |
| ----------------------------------- | ----- |
| `sizeof(p)`                         | 8     |
| `sizeof(*p)`                        | 1     |
| `sizeof(arr)`                       | 6     |
| `strlen(p)`                         | 5     |
| `strlen(arr)`                       | 5     |
| `str.size()`                        | 5     |
| `sizeof(str)` 是否等于 `str.size()` | false |

`data()` 不添加 `\0`；是 `std::string` 的存储保证在 `data()[size()]` 处有 `'\0'`（C++11 起），但 `'\0'` 不计入 `size()`，也不属于字符串内容。

`arr` 是 **6 字节**数组，`'\0'` 是数组的一部分，`sizeof(arr) == 6`。

`sizeof(str)` 计算的是 `std::string` 对象本身占用的空间 ，而 `str.size()` 计算的是它当前保存的字符数量。

### 2.注意函数参数

```c++
void func(char arr[]) {
    std::cout << sizeof(arr) << '\n';
}
// 这里的参数声明实际上等价于：void func(char* arr);
```

所以函数内部的 `sizeof(arr)` 得到的是指针大小，而不是原始数组大小。这也是为什么 Linux 系统调用需要同时传递缓冲区地址和长度：`write(fd, buf, len);`

### 3. 四种长度的区别

| 操作           | 获取的内容               |
| -------------- | ------------------------ |
| `sizeof(指针)` | 指针自身的大小           |
| `sizeof(数组)` | 整个数组占用的空间       |
| `sizeof(对象)` | 对象自身占用的空间       |
| `str.size()`   | 字符串实际保存的字符数量 |

## 4. `dup` 与共享文件偏移量

`dup()` 创建新的文件描述符，但不会创建新的打开文件描述；再次 `open()` 则会创建独立的打开文件描述。

![](/home/lgj/study/projects/mini-kv-server/docs/resource/dup.svg)

### dup()

```c++
#include <unistd.h>
// 成功时，返回一个新的文件描述符；失败时返回 -1。
int dup(int oldfd); 
```

```c++
int fd1 = open("test.txt", O_RDONLY);
int fd2 = dup(fd1);
```

此时 `fd1` 和 `fd2`：

| 属性                      | 是否共享 |
| ------------------------- | -------- |
| 文件描述符编号            | 否       |
| 文件偏移量                | 是       |
| 文件状态标志，如 O_APPEND | 是       |
| 文件内容                  | 是       |
| 文件描述符的关闭操作      | 相互独立 |

### 2次open

```c++
int fd1 = open("test.txt", O_RDONLY);
UniqueFd fd2(open("test_dup.txt", O_RDONLY));
```

### 问题

1. 在2次open的场景下：虽然两次 `open()` 的文件偏移量相互独立，但如果通过 fd1 修改了文件内容，fd2 能否读取到修改后的内容？

   - **可以。两次 `open()` 虽然拥有独立的文件偏移量，但访问的仍然是同一个文件。**

2. `fd2 = fd1` **和** `fd2 = dup(fd1)` **的本质区别**

   `fd2 = fd1`: 共享文件偏移量，但不会创建新的描述符表项，关闭其中一个 `fd` 另一个也会失效

   ![](/home/lgj/study/projects/mini-kv-server/docs/resource/fd2=fd1.svg)

​	`fd2=dup(fd1)`:
​	![](/home/lgj/study/projects/mini-kv-server/docs/resource/fd2=dup(fd1).svg)



## 5. `fcntl` 与非阻塞 I/O

### 5.1. `pipe`

```c++
#include <unistd.h>
int pipe(int pipefd[2]);   // 成功时返回 0，失败时返回 -1。  fd[1]为写端，fd[0]为读端
```

使用方式：

```c++
int fds[2];
pipe(fds);

write(fds[1], "Hello", 5);
char buf[1024];
ssize_t n = read(fds[0], buf, sizeof(buf));
```

### 5.2. `fcntl`

`fcntl()` 可以获取或修改文件描述符相关的属性。

获取文件状态标志：`int flags = fcntl(fd, F_GETFL);` 

设置非阻塞模式：

```c++
fcntl(fd, F_SETFL, flags | O_NONBLOCK);          // 按位或运算，位上有 1 就为 1
```

需要先获取原来的 flags，再通过按位或运算添加 `O_NONBLOCK`，避免丢失已有的可修改文件状态标志。

### 5.3 管道 `read`

设置完非阻塞模式后，read管道

| 操作                     | read 返回值 | errno        |
| ------------------------ | ----------- | ------------ |
| 管道为空，写端仍然打开   | -1          | EAGAIN（11） |
| 写入 Hello 后读取        | 5           | 不检查       |
| 数据已读完，写端仍然打开 | -1          | EAGAIN       |
| 写端关闭，管道为空       | 0           | 不检查       |

### 问题

1. 如果使用 `dup()` 复制管道读端，再通过 `fcntl()` 将其中一个读端设置为非阻塞模式，另一个读端会不会也变成非阻塞模式？
   - 共享。原因是 `O_NONBLOCK` 属于文件状态标志（File Status Flags），它保存在内核的打开文件描述中。

## string注意

`std::string` 内部维护字符串的实际长度，并不是依靠 `\0` 判断长度。

```c++
std::string s("AB\0CD", 5);
std::cout << s.size() << '\n';   // 5
    
std::string s("AB\0CD");         // 2  这个构造函数通过 C 风格字符串确定长度。
```

