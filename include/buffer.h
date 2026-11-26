#ifndef BUFFER_H
#define BUFFER_H

#include <cstddef>
#include <optional>
#include <string>

class Buffer {
public:
    Buffer() = default;

    void append(const char *data, std::size_t len);

    std::size_t readable_bytes() const;

    bool empty() const;

    const char *peek() const;

    void retrieve(std::size_t len);

    void retrieve_all();

    bool has_line() const;

    std::optional<std::string> retrieve_line();

private:
    std::string buffer_;
};

#endif