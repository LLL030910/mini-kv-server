#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "unique_fd.h"
#include <cstdint>

class TcpServer {
public:
    explicit TcpServer(std::uint16_t port);

    TcpServer(const TcpServer &) = delete;
    TcpServer &operator=(const TcpServer &) = delete;

    // 创建、绑定和监听 Socket
    void start();

    // 接收连接并返回 UniqueFd
    UniqueFd accept_connection();

private:
    std::uint16_t port_; // 保存服务器端口
    UniqueFd listen_fd_; // 持有监听 Socket 的所有权
};

#endif