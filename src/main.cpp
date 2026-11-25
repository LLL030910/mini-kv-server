#include "unique_fd.h"
#include <cassert>
#include <fcntl.h>
#include <iostream>

int main() {
    int raw1 = open("/dev/null", O_RDONLY);
    int raw2 = open("/dev/null", O_RDONLY);

    assert(raw1 >= 0);
    assert(raw2 >= 0);

    UniqueFd fd1(raw1);
    UniqueFd fd2(raw2);

    fd1 = std::move(fd1);
    fd1 = std::move(fd2);

    // 验证资源所有权转移
    assert(fd1.get() == raw2);
    assert(fd2.get() == -1);

    // 验证 fd1 原来的资源已被关闭
    errno = 0;
    assert(fcntl(raw1, F_GETFD) == -1);
    assert(errno == EBADF);

    // 验证接管的资源仍然有效
    assert(fcntl(raw2, F_GETFD) != -1);

    return 0;
}