#include "buffer.h"

#include <cassert>
#include <iostream>
#include <string>

void test_append() {
    Buffer buffer;

    assert(buffer.empty());

    buffer.append("Hello", 5);
    buffer.append("World", 5);

    assert(buffer.readable_bytes() == 10);

    std::string result(buffer.peek(), buffer.readable_bytes());

    assert(result == "HelloWorld");

    std::cout << "test_append passed\n";
}

void test_retrieve() {
    Buffer buffer;

    assert(buffer.empty());

    buffer.append("Hello", 5);
    buffer.append("World", 5);

    assert(buffer.readable_bytes() == 10);

    buffer.retrieve(5);

    std::string result(buffer.peek(), buffer.readable_bytes());
    assert(result == "World");

    std::cout << "test_retrieve passed\n";
}

void test_retrieve_all() {
    Buffer buffer;

    assert(buffer.empty());

    buffer.append("Hello", 5);
    buffer.append("World", 5);

    assert(buffer.readable_bytes() == 10);

    buffer.retrieve_all();

    assert(buffer.readable_bytes() == 0);
    assert(buffer.empty());

    std::cout << "test_retrieve_all passed\n";
}

void test_binary_data() {
    Buffer buffer;

    assert(buffer.empty());

    buffer.append("AB\0CD", 5);

    assert(buffer.readable_bytes() == 5);

    std::string result(buffer.peek(), buffer.readable_bytes());
    assert(result == std::string("AB\0CD", 5));

    std::cout << "test_binary_data passed\n";
}

void test_invalid_retrieve() {
    Buffer buffer;

    assert(buffer.empty());

    buffer.append("Hello", 5);

    assert(buffer.readable_bytes() == 5);

    bool caught = false;
    try {
        buffer.retrieve(6);
    } catch (const std::out_of_range &e) {
        caught = true;
    }
    assert(caught);
    assert(buffer.readable_bytes() == 5);
    assert(std::string(buffer.peek(), 5) == "Hello");

    std::cout << "test_invalid_retrieve passed\n";
}

void test_retrieve_boundary() {
    Buffer buffer;
    buffer.append("Hello", 5);

    buffer.retrieve(0);
    assert(buffer.readable_bytes() == 5);

    buffer.retrieve(5);
    assert(buffer.empty());

    buffer.retrieve(0);
    assert(buffer.empty());

    std::cout << "test_retrieve_boundary passed\n";
}

void test_retrieve_line() {
    Buffer buffer;

    buffer.append("Hello\n", 6);

    assert(buffer.has_line());

    auto result = buffer.retrieve_line();

    assert(result.has_value());
    assert(result.value() == "Hello");

    assert(buffer.empty());
    assert(!buffer.has_line());

    std::cout << "test_retrieve_line passed\n";
}

void test_partial_line() {
    Buffer buffer;

    buffer.append("Hel", 3);

    assert(!buffer.has_line());

    auto result = buffer.retrieve_line();
    assert(!result.has_value());
    assert(buffer.readable_bytes() == 3);

    buffer.append("lo\n", 3);

    assert(buffer.has_line());
    result = buffer.retrieve_line();
    assert(result == "Hello");

    assert(buffer.empty());
    assert(!buffer.has_line());

    std::cout << "test_partial_line passed\n";
}

void test_multiple_lines() {
    Buffer buffer;

    buffer.append("Hello\nWorld\n", 12);

    assert(buffer.has_line());
    auto result = buffer.retrieve_line();
    assert(result.has_value());
    assert(result.value() == "Hello");

    assert(buffer.has_line());
    result = buffer.retrieve_line();
    assert(result.has_value());
    assert(result.value() == "World");

    assert(buffer.empty());
    assert(!buffer.has_line());

    std::cout << "test_multiple_lines passed\n";
}

void test_remaining_data() {
    Buffer buffer;

    buffer.append("Hello\nWor", 9);

    assert(buffer.has_line());
    auto result = buffer.retrieve_line();
    assert(result.has_value());
    assert(result.value() == "Hello");

    assert(!buffer.has_line());
    result = buffer.retrieve_line();
    assert(!result.has_value());

    assert(!buffer.empty());
    assert(!buffer.has_line());

    std::cout << "test_remaining_data passed\n";
}

void test_empty_line() {
    Buffer buffer;

    buffer.append("\n", 1);

    assert(buffer.has_line());
    auto result = buffer.retrieve_line();
    assert(result.has_value());
    assert(result.value() == "");

    assert(buffer.empty());
    assert(!buffer.has_line());

    std::cout << "test_empty_line passed\n";
}

void test_no_data() {
    Buffer buffer;

    assert(!buffer.has_line());
    auto result = buffer.retrieve_line();
    assert(!result.has_value());

    assert(buffer.empty());
    assert(!buffer.has_line());

    std::cout << "test_no_data passed\n";
}

int main() {
    // test_append();
    // test_retrieve();
    // test_retrieve_all();
    // test_binary_data();
    // test_invalid_retrieve();
    // test_retrieve_boundary();
    test_retrieve_line();
    test_partial_line();
    test_multiple_lines();
    test_remaining_data();
    test_empty_line();
    test_no_data();
}