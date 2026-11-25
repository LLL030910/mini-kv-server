#include "unique_fd.h"
#include <iostream>
#include <unistd.h>
#include <utility>

UniqueFd::UniqueFd(int fd) noexcept : fd_(fd) {
    std::cout << "UniqueFd(int fd) is called\n";
}

UniqueFd::UniqueFd(UniqueFd &&other) noexcept
    : fd_(std::exchange(other.fd_, -1)) {
    std::cout << "UniqueFd(UniqueFd &&other) is called\n";
}
UniqueFd &UniqueFd::operator=(UniqueFd &&other) noexcept {
    std::cout << "operator=(UniqueFd &&other) is called\n";
    if (&other != this) {
        if (this->fd_ >= 0)
            close(this->fd_);
        this->fd_ = std::exchange(other.fd_, -1);
    }
    return *this;
}

UniqueFd::~UniqueFd() noexcept {
    std::cout << "~UniqueFd() is called\n";
    if (fd_ >= 0)
        close(fd_);
}

int UniqueFd::get() const noexcept { return fd_; }