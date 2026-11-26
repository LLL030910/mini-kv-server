#include "buffer.h"
#include <iostream>
#include <stdexcept>

void Buffer::append(const char *data, std::size_t len) {
    buffer_.append(data, len);
}

std::size_t Buffer::readable_bytes() const { return buffer_.size(); }

bool Buffer::empty() const { return buffer_.empty(); }

const char *Buffer::peek() const { return buffer_.data(); }

void Buffer::retrieve(std::size_t len) {
    if (len > buffer_.size()) {
        throw std::out_of_range("retrieve len > readable bytes");
    }
    buffer_.erase(0, len);
}

void Buffer::retrieve_all() { buffer_.clear(); }

bool Buffer::has_line() const {
    return buffer_.find('\n') != std::string::npos;
}

std::optional<std::string> Buffer::retrieve_line() {
    std::size_t pos = buffer_.find('\n');
    if (pos == std::string::npos) {
        return std::nullopt;
    }
    std::string message = buffer_.substr(0, pos);
    // pos 是从 0 开始的， 所以需要加 1
    retrieve(pos + 1);
    return message;
}