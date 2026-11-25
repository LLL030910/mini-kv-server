## 1. Socket 文件描述符

监听 Socket 可以持续接收连接，每次成功 accept 都会返回一个新的连接 Socket。

## 2. socket API

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

### 6. `send()`

```c++
#include <sys/socket.h>

ssize_t send(int sockfd, const void* buf, size_t len, int flags);
```

如果连接已经不可写，继续发送数据可能产生 `SIGPIPE` 信号。`SIGPIPE` 的默认处理行为是终止进程。对于服务器来说，一个客户端异常断开，不应该导致整个服务器退出。

可以使用：

```c++
send(fd, data, size, MSG_NOSIGNAL);
```

这样可以抑制本次发送产生的 `SIGPIPE`。如果出现 `EPIPE` 等错误，仍然需要根据返回值进行处理。

### 7. `recv()`

当为阻塞模式下有3种情况：

- 内核接收缓冲区有数据，把数据读取完并返回读取长度
- **暂时没有数据，连接仍然存在**，阻塞等待
- **对端关闭发送方向，数据已读完**，recv() 返回 0

## 3. 网络命令

### 1. ss(Socket Statistics)

`ss -lntp` 只看处于“监听”状态的端口，而 `ss -ntp` 看的是所有 TCP 连接（包括监听和已建立的连接）。

参数：

​	**`-n`**：以数字形式显示端口和地址，不进行 DNS 或服务名解析，速度更快。

​	**`	-t`**：仅显示 TCP 协议的套接字。

​	**`	-p`**：显示占用该套接字的进程（PID 和程序名），通常需要 root 权限才能看到其他用户的进程。

​	**`-l`**：关键区别。仅显示处于 `LISTEN`（监听）状态的套接字



服务端启动，首先客户端 `A` 连接，但不发送消息，紧接客户端 `B` 连接，并发送数据 `Hello from B`  ，然后如下：

```shell
lgj@R7000P-lgj:~/study/projects/mini-kv-server$ ss -ntp | grep 8080
 状态    Recv-Q  Send-Q                                本地地址和端口                 对端地址和端口
ESTAB      0      0                               127.0.0.1:51640(B)              127.0.0.1:8080  users:(("nc",pid=30452,fd=3)) 
ESTAB      0      0                               127.0.0.1:56066(A)             127.0.0.1:8080  users:(("nc",pid=30366,fd=3)) 
ESTAB      13     0                                   127.0.0.1:8080               127.0.0.1:51640  
ESTAB      0      0                               127.0.0.1:8080        127.0.0.1:56066users(("test_tcp_server",pid=30329,fd=4))
```

**为什么客户端 B 对应的服务器端 Recv-Q 是 13？**

- 户端 B 已经成功建立 TCP 连接。
- 客户端 B 发送的 13 字节**已经到达服务器内核**。
- 服务器应用程序还没有读取这些数据。
- 服务器还没有通过 `accept()` 获得该连接对应的文件描述符。

### 2. nc IP Port

```shell
nc 127.0.0.1 8080 
```

会建立一个到本机 8080 端口的 TCP 连接。如果端口开放，你的终端会进入一个“透传”模式：你输入什么，就会原样发给 8080 端口的服务；服务返回什么，也会原样显示出来。