#include "append_log.h"
#include <fcntl.h>
#include <io_utils.h>
#include <string>
#include <system_error>
#include <unistd.h>

AppendLog::AppendLog(const std::string &path)
    : fd_(open(path.c_str(), O_CREAT | O_RDWR | O_APPEND, 0644)) {
    if (fd_.get() < 0) {
        throw std::system_error(errno, std::generic_category(), "open" + path);
    }
}

void AppendLog::append(const std::string &record) {
    // 由于构造函数已经使用：O_CREAT | O_RDWR | O_APPEND
    // 所以每次调用 write() 时，内核都会自动将写入位置定位到文件末尾。

    write_all(fd_.get(), record.data(), record.length());
    write_all(fd_.get(), "\n", 1);
}

std::string AppendLog::read_all() const {
    // 将文件偏移量移动至开头
    off_t offset = lseek(fd_.get(), 0, SEEK_SET);
    if (offset == -1) {
        throw std::system_error(errno, std::generic_category(), "lseek");
    }

    return read_to_end(fd_.get());
}