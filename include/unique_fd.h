#ifndef UNIQUE_FD_H
#define UNIQUE_FD_H

class UniqueFd {
public:
    explicit UniqueFd(int fd = -1) noexcept;
    ~UniqueFd() noexcept;
    UniqueFd(const UniqueFd &) = delete;
    UniqueFd &operator=(const UniqueFd &) = delete;

    UniqueFd(UniqueFd &&other) noexcept;
    UniqueFd &operator=(UniqueFd &&other) noexcept;

    int get() const noexcept;

private:
    int fd_;
};

#endif