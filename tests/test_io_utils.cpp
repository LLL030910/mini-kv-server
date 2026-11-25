#include "io_utils.h"
#include "unique_fd.h"

#include <cassert>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <system_error>
#include <unistd.h>

void test_basic_io() {
    UniqueFd fd(open("test_io.txt", O_CREAT | O_RDWR | O_TRUNC, 0644));

    assert(fd.get() >= 0);

    std::string data = "Hello Linux";

    write_all(fd.get(), data.data(), data.size());

    // 将文件偏移量移动至开头
    off_t offset = lseek(fd.get(), 0, SEEK_SET);
    assert(offset == 0);

    std::string result = read_to_end(fd.get());

    assert(result == data);

    std::cout << "test_basic_io passed\n";
}

void test_large_data() {
    UniqueFd fd(open("test_large.txt", O_CREAT | O_RDWR | O_TRUNC, 0644));

    assert(fd.get() >= 0);

    std::string data(5000, 'A');

    write_all(fd.get(), data.data(), data.size());

    // 将文件偏移量移动至开头
    off_t offset = lseek(fd.get(), 0, SEEK_SET);
    assert(offset == 0);

    std::string result = read_to_end(fd.get());

    assert(result.size() == 5000);
    assert(result == data);

    std::cout << "test_large_data passed\n";
}

void test_binary_data() {
    UniqueFd fd(open("test_binary.txt", O_CREAT | O_RDWR | O_TRUNC, 0644));

    assert(fd.get() >= 0);

    std::string data("AB\0CD", 5);

    write_all(fd.get(), data.data(), data.size());

    // 将文件偏移量移动至开头
    off_t offset = lseek(fd.get(), 0, SEEK_SET);
    assert(offset == 0);

    std::string result = read_to_end(fd.get());

    assert(result.size() == 5);
    assert(result == data);

    std::cout << "test_binary_data passed\n";
}

void test_empty() {
    UniqueFd fd(open("test_empty.txt", O_CREAT | O_RDWR | O_TRUNC, 0644));

    assert(fd.get() >= 0);

    std::string result = read_to_end(fd.get());

    assert(result.size() == 0);
    assert(result.empty());

    std::cout << "test_empty passed\n";
}

void test_invalid_fd() {
    bool caught = false;

    // 测试 read_to_end
    try {
        read_to_end(-1);
    } catch (const std::system_error &e) {
        caught = true;
        assert(e.code().value() == EBADF);
    }

    assert(caught);

    // 测试 write_all
    caught = false;

    try {
        write_all(-1, "ABC", 3);
    } catch (const std::system_error &e) {
        caught = true;
        assert(e.code().value() == EBADF);
    }

    assert(caught);

    std::cout << "test_invalid_fd passed\n";
}

void test_zero_length() {
    UniqueFd fd(open("test_zero_length.txt", O_CREAT | O_RDWR | O_TRUNC, 0644));

    assert(fd.get() >= 0);

    std::string data = "ABC";

    write_all(fd.get(), data.data(), data.size());

    off_t before = lseek(fd.get(), 0, SEEK_CUR);

    write_all(fd.get(), "", 0);

    off_t after = lseek(fd.get(), 0, SEEK_CUR);

    assert(before == after);

    // 将文件偏移量移动至开头
    off_t offset = lseek(fd.get(), 0, SEEK_SET);
    assert(offset == 0);
    std::string result = read_to_end(fd.get());

    assert(result.size() == data.size());
    assert(result == data);

    std::cout << "test_zero_length passed\n";
}

int main() {
    test_basic_io();
    test_large_data();
    test_binary_data();
    test_empty();
    test_invalid_fd();
    test_zero_length();
}