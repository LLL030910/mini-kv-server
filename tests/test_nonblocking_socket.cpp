#include "io_utils.h"
#include "unique_fd.h"

#include <cassert>
#include <cerrno>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <system_error>

void test_nonblocking_socket() {
    int fds[2];

    // 1：使用 socketpair 创建两个 Socket
    int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
    // 检查返回值
    if (ret == -1) {
        throw std::system_error(errno, std::generic_category(), "socketpair");
    }

    UniqueFd receiver(fds[0]);

    char buf[1024];
    ssize_t n;

    {
        UniqueFd sender(fds[1]);

        // 2：将 receiver 设置为非阻塞模式
        set_nonblocking(receiver.get());

        // 3：调用 recv，验证没有数据时
        n = recv(receiver.get(), buf, sizeof(buf), 0);
        // 返回 -1，errno 为 EAGAIN 或 EWOULDBLOCK
        assert(n == -1 && errno == EAGAIN);

        // 4：使用 send_all 发送 "Hello"
        send_all(sender.get(), "Hello", 5);

        // 5：调用 recv，验证收到的数据
        n = recv(receiver.get(), buf, sizeof(buf), 0);
        assert(n == 5 && std::string(buf, n) == "Hello");

        // 6：再次调用 recv
        n = recv(receiver.get(), buf, sizeof(buf), 0);
        // 验证接收缓冲区为空时的返回值
        assert(n == -1 && errno == EAGAIN);
    } // sender 自动析构

    // 7：再次调用 recv
    n = recv(receiver.get(), buf, sizeof(buf), 0);
    // 验证对端关闭后的返回值
    assert(n == 0);

    std::cout << "test_nonblocking_socket passed\n";
}

int main() { test_nonblocking_socket(); }