#include "command_handler.h"
#include <sstream>

std::string handle_command(KVStore &store, const std::string &command) {
    std::istringstream iss(command);

    std::string cmd, extra;
    std::string key, value;

    // 1：提取命令，处理空消息
    if (!(iss >> cmd)) {
        return "ERR\n";
    }

    if (cmd == "PING") {
        // 2：检查是否存在多余参数
        if (iss >> extra) {
            return "ERR\n";
        }
        // 返回 PONG\n
        return "PONG\n";

    } else if (cmd == "SET") {
        // 3：提取 key 和 value
        if (!(iss >> key >> value)) {
            return "ERR\n";
        }
        // 检查参数数量
        if (iss >> extra) {
            return "ERR\n";
        }
        // 调用 store.set()
        store.set(key, value);
        // 返回 OK\n
        return "OK\n";

    } else if (cmd == "GET") {
        // 4：提取 key
        if (!(iss >> key)) {
            return "ERR\n";
        }
        // 检查参数数量
        if (iss >> extra) {
            return "ERR\n";
        }
        // 调用 store.get()
        auto res = store.get(key);
        // 根据查询结果返回响应
        if (res.has_value()) {
            return *res + "\n";
        } else {
            return "NOT_FOUND\n";
        }

    } else if (cmd == "DEL") {
        // 5：提取 key
        if (!(iss >> key)) {
            return "ERR\n";
        }
        // 检查参数数量
        if (iss >> extra) {
            return "ERR\n";
        }
        // 调用 store.del()
        bool res = store.del(key);
        // 返回 1\n 或 0\n
        return res ? "1\n" : "0\n";
    }

    return "ERR\n";
}