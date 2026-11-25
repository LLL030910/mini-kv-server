#include "io_utils.h"
#include "unique_fd.h"

#include <cassert>
#include <cerrno>
#include <fcntl.h>
#include <iostream>
#include <system_error>
#include <unistd.h>

// void set_nonblocking(int fd) {
//     int flags = fcntl(fd, F_GETFL);
//     if (flags == -1) {
//         throw std::system_error(errno, std::generic_category(),
//                                 "fcntl F_GETFL");
//     }
//     if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
//         throw std::system_error(errno, std::generic_category(),
//                                 "fcntl F_SETFL");
//     }
// }

void test_nonblocking_pipe() {
    int fds[2];

    if (pipe(fds) == -1) {
        throw std::system_error(errno, std::generic_category(), "pipe");
    }

    UniqueFd read_fd(fds[0]);
    char buf[1024];
    ssize_t n;

    {
        UniqueFd write_fd(fds[1]);

        set_nonblocking(read_fd.get());

        // 1：管道为空时调用 read()
        n = read(read_fd.get(), buf, sizeof(buf));
        assert(n == -1);
        assert(errno == EAGAIN || errno == EWOULDBLOCK);

        // 2：向管道写入 "Hello"
        write_all(write_fd.get(), "Hello", 5);

        // 3：再次调用 read()
        n = read(read_fd.get(), buf, sizeof(buf));
        // 输出实际读取的数据
        assert(n == 5);
        assert(std::string(buf, n) == "Hello");

        // 4：再次读取已经为空的管道
        n = read(read_fd.get(), buf, sizeof(buf));
        assert(n == -1);
        assert(errno == EAGAIN || errno == EWOULDBLOCK);

    } // write_fd 自动析构，关闭写端

    // 5：再次调用 read()
    n = read(read_fd.get(), buf, sizeof(buf));
    assert(n == 0);

    std::cout << "test_nonblocking_pipe passed\n";
}

int main() { test_nonblocking_pipe(); }