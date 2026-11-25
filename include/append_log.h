#ifndef APPEND_LOG_H
#define APPEND_LOG_H

#include "unique_fd.h"
#include <string>

class AppendLog {
public:
    explicit AppendLog(const std::string &path);

    void append(const std::string &record);

    std::string read_all() const;

private:
    UniqueFd fd_;
};

#endif