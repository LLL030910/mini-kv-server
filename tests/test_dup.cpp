#include "io_utils.h"
#include "unique_fd.h"

#include <cassert>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

void test_dup_offset() {
    UniqueFd fd1(open("test_dup.txt", O_CREAT | O_RDWR | O_TRUNC, 0644));

    assert(fd1.get() >= 0);

    write_all(fd1.get(), "ABCDE12345", 10);

    off_t offset = lseek(fd1.get(), 0, SEEK_SET);
    assert(offset == 0);

    // 1：使用 dup() 创建第二个 UniqueFd
    UniqueFd fd2(dup(fd1.get()));

    // 2：打印 fd1 和 fd2 的编号
    std::cout << "fd1 : " << fd1.get() << std::endl;
    std::cout << "fd2 : " << fd2.get() << std::endl;

    // 3：使用 fd1 读取 3 字节
    char buf[1024];
    ssize_t n = read(fd1.get(), buf, 3);
    if (n > 0) {
        std::cout.write(buf, n);
        std::cout << '\n';
    }

    // 4：使用 fd2 读取 2 字节
    n = read(fd2.get(), buf, 2);
    if (n > 0) {
        std::cout.write(buf, n);
        std::cout << '\n';
    }

    // 5：再次使用 fd1 读取 2 字节
    n = read(fd1.get(), buf, 2);
    if (n > 0) {
        std::cout.write(buf, n);
        std::cout << '\n';
    }

    // 6：分别打印两个 fd 当前的偏移量
    offset = lseek(fd1.get(), 0, SEEK_CUR);
    std::cout << "fd1 offset : " << offset << std::endl;
    offset = lseek(fd2.get(), 0, SEEK_CUR);
    std::cout << "fd2 offset : " << offset << std::endl;
}

void test_independent_offset() {
    UniqueFd fd1(open("test_dup.txt", O_CREAT | O_RDWR | O_TRUNC, 0644));

    assert(fd1.get() >= 0);

    write_all(fd1.get(), "ABCDE12345", 10);

    off_t offset = lseek(fd1.get(), 0, SEEK_SET);
    assert(offset == 0);

    UniqueFd fd2(open("test_dup.txt", O_RDONLY));

    // 2：打印 fd1 和 fd2 的编号
    std::cout << "fd1 : " << fd1.get() << std::endl;
    std::cout << "fd2 : " << fd2.get() << std::endl;

    // 3：使用 fd1 读取 3 字节
    char buf[1024];

    ssize_t n = read(fd1.get(), buf, 3);
    assert(n == 3);
    assert(std::string(buf, n) == "ABC");
    std::cout.write(buf, n);
    std::cout << '\n';

    n = read(fd2.get(), buf, 2);
    assert(n == 2);
    assert(std::string(buf, n) == "AB");
    std::cout.write(buf, n);
    std::cout << '\n';

    n = read(fd1.get(), buf, 2);
    assert(n == 2);
    assert(std::string(buf, n) == "DE");
    std::cout.write(buf, n);
    std::cout << '\n';

    assert(lseek(fd1.get(), 0, SEEK_CUR) == 5);
    assert(lseek(fd2.get(), 0, SEEK_CUR) == 2);
    std::cout << "test independent offset pass" << std::endl;

    // fd1 移动到文件开头
    offset = lseek(fd1.get(), 0, SEEK_SET);
    assert(offset == 0);

    // 修改文件前两个字节
    write_all(fd1.get(), "XY", 2);

    // 验证 fd2 的偏移量没有受到影响
    assert(lseek(fd2.get(), 0, SEEK_CUR) == 2);

    // fd2 移动到文件开头
    offset = lseek(fd2.get(), 0, SEEK_SET);
    assert(offset == 0);

    // 通过 fd2 读取文件
    std::string result = read_to_end(fd2.get());

    std::cout << result << '\n';

    assert(result == "XYCDE12345");
}

void test_dup_lifetime() {
    UniqueFd fd2;
    char buf[1024];

    {
        UniqueFd fd1(open("test_dup.txt", O_RDONLY));

        assert(fd1.get() >= 0);

        fd2 = UniqueFd(dup(fd1.get()));

        assert(fd2.get() >= 0);

        std::cout << "fd1: " << fd1.get() << '\n';
        std::cout << "fd2: " << fd2.get() << '\n';

        // 1：使用 fd1 读取 3 字节
        ssize_t n = read(fd1.get(), buf, 3);
        assert(n == 3);
        assert(std::string(buf, n) == "ABC");
        std::cout.write(buf, n);
        std::cout << '\n';

        std::cout << "Leaving inner scope\n";
    }

    // 此时 fd1 已经析构

    std::cout << "fd1 has been closed\n";

    // 2：检查 fd2 当前的偏移量
    off_t offset = lseek(fd2.get(), 0, SEEK_CUR);
    std::cout << "fd2 : " << offset << std::endl;

    // 3：通过 fd2 读取剩余的文件内容
    std::string result = read_to_end(fd2.get());

    // 4：验证读取结果
    assert(result == "DE12345");
}

int main() {
    // test_dup_offset();
    // test_independent_offset();
    test_dup_lifetime();
}