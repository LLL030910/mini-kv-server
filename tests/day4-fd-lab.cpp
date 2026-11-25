#include <cstdio>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

int main() {
    int fd = open("test.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);

    if (fd == -1) {
        perror("open");
        return 1;
    }

    std::cout << "fd = " << fd << '\n';

    const char *msg = "Hello Linux";

    ssize_t n = write(fd, msg, 11);
    std::cout << "written = " << n << '\n'; // 11

    std::cout << "offset = " << lseek(fd, 0, SEEK_CUR) << '\n'; // 11

    char buf[128] = {};

    n = read(fd, buf, sizeof(buf));
    std::cout << "first read = " << n << '\n'; // 0?
    if (n == -1) {
        perror("read");
    }

    lseek(fd, 0, SEEK_SET);

    n = read(fd, buf, sizeof(buf));
    std::cout << "second read = " << n << '\n'; // 11?
    if (n == -1) {
        perror("read");
    }

    if (n > 0) {
        std::cout.write(buf, n);
        std::cout << '\n';
    }

    close(fd);
}