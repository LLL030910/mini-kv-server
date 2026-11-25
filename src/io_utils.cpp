#include "io_utils.h"
#include <fcntl.h>
#include <sys/socket.h>
#include <system_error>
#include <unistd.h>

void write_all(int fd, const void *data, std::size_t len) {
    const char *buf = static_cast<const char *>(data);
    size_t total = 0;
    while (total < len) {
        ssize_t n = write(fd, buf + total, len - total);

        if (n > 0) {
            total += static_cast<std::size_t>(n);
        } else if (n == -1) {
            if (errno == EINTR) {
                continue;
            }

            throw std::system_error(errno, std::generic_category(), "write");
        } else {
            throw std::runtime_error("write returned zero");
        }
    }
}

std::string read_to_end(int fd) {
    std::string result;
    char buf[1024];
    while (true) {
        ssize_t n = read(fd, buf, sizeof(buf));

        if (n > 0) {
            result.append(buf, static_cast<std::size_t>(n));
        } else if (n == -1) {
            if (errno == EINTR) {
                continue;
            }
            throw std::system_error(errno, std::generic_category(), "read");
        } else {
            break;
        }
    }

    return result;
}

void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL);
    if (flags == -1) {
        throw std::system_error(errno, std::generic_category(),
                                "fcntl F_GETFL");
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        throw std::system_error(errno, std::generic_category(),
                                "fcntl F_SETFL");
    }
}

void send_all(int fd, const void *data, std::size_t size) {
    const char *buf = static_cast<const char *>(data);
    size_t total = 0;
    while (total < size) {
        ssize_t n = send(fd, buf + total, size - total, MSG_NOSIGNAL);

        if (n > 0) {
            total += static_cast<std::size_t>(n);
        } else if (n == -1) {
            if (errno == EINTR) {
                continue;
            }

            throw std::system_error(errno, std::generic_category(), "send");
        } else {
            throw std::runtime_error("send returned zero");
        }
    }
}