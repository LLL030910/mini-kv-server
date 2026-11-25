#include <iostream>
#include <string>
#include <vector>

class Tracer {
public:
    // 不允许隐式地将 std::string 转换为 Tracer
    explicit Tracer(std::string name) : name_(std::move(name)) {
        std::cout << "Constructor: " << name_ << '\n';
    }

    Tracer(const Tracer &other) : name_(other.name_) {
        std::cout << "Copy: " << name_ << '\n';
    }

    Tracer(Tracer &&other) : name_(std::move(other.name_)) {
        std::cout << "Move: " << name_ << '\n';
    }

private:
    std::string name_;
};

int main() {
    std::vector<Tracer> vec;

    vec.reserve(1);
    vec.emplace_back("A");

    std::cout << "---- Add B ----\n";

    vec.emplace_back("B");
}