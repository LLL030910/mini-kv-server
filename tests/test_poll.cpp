#include "io_utils.h"
#include <cassert>
#include <iostream>
#include <poll.h>
#include <sys/socket.h>
#include <unique_fd.h>

int main() {
    int fds1[2];
    int fds2[2];

    char buf[1024];

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds1) == -1) {
        throw std::system_error(errno, std::generic_category(), "socketpair");
    }
    UniqueFd socket_a(fds1[0]);
    UniqueFd sender_a(fds1[1]);
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds2) == -1) {
        throw std::system_error(errno, std::generic_category(), "socketpair");
    }
    UniqueFd socket_b(fds2[0]);
    UniqueFd sender_b(fds2[1]);

    set_nonblocking(socket_a.get());
    set_nonblocking(socket_b.get());

    pollfd fds[2]{};

    fds[0].fd = socket_a.get();
    fds[0].events = POLLIN;

    fds[1].fd = socket_b.get();
    fds[1].events = POLLIN;

    // 第二步：第一次调用 poll
    int ret = poll(fds, 2, 1000);
    assert(ret == 0);

    // 第三步：只向 Socket B 发送数据
    send_all(sender_b.get(), "PING\n", 5);
    ret = poll(fds, 2, 1000);
    assert(ret == 1);

    // B 必须有 POLLIN 事件
    assert((fds[1].revents & POLLIN) != 0);
    if (fds[1].revents & POLLIN) {
        ssize_t n = recv(socket_b.get(), buf, sizeof(buf), 0);

        assert(n == 5);
        assert(std::string(buf, n) == "PING\n");
    }

    // A 不应该有 POLLIN 事件
    assert((fds[0].revents & POLLIN) == 0);
}