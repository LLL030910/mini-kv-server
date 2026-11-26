#ifndef KV_STORE_H
#define KV_STORE_H

#include <optional>
#include <string>
#include <unordered_map>

class KVStore {
public:
    // 插入或更新键值对
    void set(const std::string &key, const std::string &value);

    // 根据 key 获取 value
    std::optional<std::string> get(const std::string &key) const;

    // 删除键值对
    bool del(const std::string &key);

    // 获取当前键值对数量
    std::size_t size() const;

private:
    std::unordered_map<std::string, std::string> data_;
};

#endif