## Socket 文件描述符

监听 Socket 可以持续接收连接，每次成功 accept 都会返回一个新的连接 Socket。

## socket API

### 1. `socket()`

```c++
#include <sys/socket.h>

int socket(int domain, int type, int protocol);
```

创建一个 IPv4 TCP Socket：

```c++
// AF_INET: IPv4 网络协议
// SOCK_STREAM: 面向连接的字节流 Socket
int fd = socket(AF_INET, SOCK_STREAM, 0);
```

### 2. `setsockopt()`

```c++
int setsockopt(int socket, int level, int option_name, const void *option_value, socklen_t option_len);
```

**`level`**：选项所在的协议层级。对于通用套接字选项（如 `SO_REUSEADDR`），使用 `SOL_SOCKET`；对于 TCP 协议选项（如 `TCP_NODELAY`），使用 `IPPROTO_TCP`

**`option_name`**：要设置的选项名称。如 `SO_REUSEADDR,` 

使用方法：

```c++
int opt = 1;
setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));  // 开启端口复用

int opt = 0;
setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));  // 关闭端口复用(默认就是关)
```

### 3. `bind()`

```c++
#include <netinet/in.h>
#include <arpa/inet.h>

sockaddr_in addr{};

addr.sin_family = AF_INET;                       // 地址族
addr.sin_port = htons(8080);                     // 端口号
addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);   // IPv4 地址 仅监听本机 127.0.0.1

int ret = bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
```

`htons`: 16 位主机字节序转网络字节序； `htonl`: 32 位主机字节序转网络字节序

### 4. `listen()`

```c++
int ret = listen(fd, SOMAXCONN);              // SOMAXCONN 用于指定监听队列的 backlog 参数，实际限制由操作系统决定。
```

### 5. `accept()`

```c++
sockaddr_in client_addr{};
socklen_t client_len = sizeof(client_addr);

int connfd = accept(fd, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
```

当监听队列中没有可接收的连接时，阻塞式 `accept()` 会使调用线程进入睡眠等待状态，操作系统调度其他可运行的线程。当新连接就绪时，内核会唤醒等待的线程。因此，它不会持续占用 CPU 进行轮询。

非阻塞 `accept` 通过 `fcntl` 设置

| 方式                           | 没有客户端连接时的表现        |
| ------------------------------ | ----------------------------- |
| 阻塞 `accept`                  | 线程睡眠等待                  |
| 非阻塞 `accept` + `while` 循环 | 不断调用，可能大量消耗 CPU    |
| 非阻塞 `accept` + `epoll`      | 等待就绪事件，再执行 `accept` |

**打印客户端地址：**

`inet_ntop`: 将网络字节序的二进制 IP 地址转换为人类可读的文本字符串（如 `"127.0.0.1"`）

```c++
char ip[INET_ADDRSTRLEN]{};

if (inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip)) == nullptr) {
    throw std::system_error(
        errno, std::generic_category(), "inet_ntop");
}
std::cout << ip << ":" << ntohs(client_addr.sin_port) << std::endl;
```

## 网络命令

### ss(Socket Statistics)

`ss -lntp` 只看处于“监听”状态的端口，而 `ss -ntp` 看的是所有 TCP 连接（包括监听和已建立的连接）。

参数：

​	**`-n`**：以数字形式显示端口和地址，不进行 DNS 或服务名解析，速度更快。

​	**`	-t`**：仅显示 TCP 协议的套接字。

​	**`	-p`**：显示占用该套接字的进程（PID 和程序名），通常需要 root 权限才能看到其他用户的进程。

​	**`-l`**：关键区别。仅显示处于 `LISTEN`（监听）状态的套接字。