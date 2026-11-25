#include "connection_manager.h"

#include <cassert>
#include <iostream>
#include <memory>

int main() {
    ConnectionManager manager;

    manager.add(1);
    manager.add(1); // 重复添加

    std::cout << "size : " << manager.size() << '\n';

    auto conn = manager.get(1);

    std::cout << "Count = " << conn.use_count() << '\n';

    manager.remove(1);

    std::cout << "Count = " << conn.use_count() << '\n';

    conn.reset();

    std::cout << "End\n";
}