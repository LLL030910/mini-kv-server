#include "tcp_server.h"
#include <arpa/inet.h>
#include <cerrno>
#include <iostream>
#include <sys/socket.h>
#include <system_error>
#include <utility>

TcpServer::TcpServer(std::uint16_t port) : port_(port) {}

// 创建、绑定和监听 Socket
void TcpServer::start() {
    if (listen_fd_.get() >= 0) {
        std::cout << "TCP Server already started..." << std::endl;
        return;
    }

    // 1. 创建 TCP Socket
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        throw std::system_error(errno, std::generic_category(), "socket");
    }
    // 2. 使用 UniqueFd 管理 Socket
    UniqueFd listen_fd(fd);

    // 3. 设置 SO_REUSEADDR
    int opt = 1;
    int ret = setsockopt(listen_fd.get(), SOL_SOCKET, SO_REUSEADDR, &opt,
                         sizeof(opt));
    if (ret == -1) {
        throw std::system_error(errno, std::generic_category(), "setsockopt");
    }
    // 4. 初始化 sockaddr_in
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    // 5. bind()
    if (bind(listen_fd.get(), reinterpret_cast<const sockaddr *>(&addr),
             sizeof(addr)) == -1) {
        throw std::system_error(errno, std::generic_category(), "bind");
    }

    // 6. listen()
    if (listen(listen_fd.get(), SOMAXCONN) == -1) {
        throw std::system_error(errno, std::generic_category(), "listen");
    }

    // 7. 将监听 Socket 的所有权移交给成员变量 listen_fd_
    listen_fd_ = std::move(listen_fd);

    std::cout << "=== TCP Server start success ===" << std::endl;
}

// 接收连接并返回 UniqueFd
UniqueFd TcpServer::accept_connection() {
    if (listen_fd_.get() < 0) {
        throw std::logic_error("TCP Server is not started");
    }

    while (true) {
        // 1. 定义客户端地址结构
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);

        // 2. accept
        int connfd =
            accept(listen_fd_.get(), reinterpret_cast<sockaddr *>(&client_addr),
                   &client_len);
        if (connfd == -1) {
            if (errno == EINTR) {
                continue;
            }
            throw std::system_error(errno, std::generic_category(), "accept");
        }
        // 成功 accept 后应该立即接管 fd
        UniqueFd client_fd(connfd);

        // 打印客户端 IP 和端口
        char ip[INET_ADDRSTRLEN] = {};
        if (inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip)) ==
            nullptr) {
            throw std::system_error(errno, std::generic_category(),
                                    "inet_ntop");
        }
        std::cout << ip << ":" << ntohs(client_addr.sin_port) << std::endl;

        return client_fd;
    }
}