#include <iostream>
#include <string>
#include <utility>

void process(const std::string &str) { std::cout << "Lvalue version\n"; }

void process(std::string &&str) { std::cout << "Rvalue version\n"; }

template <typename T> void wrapper(T &&value) { process(std::move(value)); }

int main() {
    std::string name = "Linux";

    wrapper(name);                  // rvalue
    wrapper(std::string("Server")); // rvalue
}