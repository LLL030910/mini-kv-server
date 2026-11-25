#include "append_log.h"
#include <iostream>

int main() {
    AppendLog log("aof.log");

    log.append("SET name Alice");
    log.append("SET age 20");

    std::cout << log.read_all();

    log.append("SET name Bob");

    std::cout << "\nAfter append:\n";
    std::cout << log.read_all();

    return 0;
}