#include "kv_store.h"

// 插入或更新键值对
void KVStore::set(const std::string &key, const std::string &value) {
    data_.insert_or_assign(key, value);
}

// 根据 key 获取 value
std::optional<std::string> KVStore::get(const std::string &key) const {
    auto it = data_.find(key);
    if (it == data_.end()) {
        return std::nullopt;
    }
    return it->second;
}

// 删除键值对
bool KVStore::del(const std::string &key) {
    // auto it = data_.find(key);
    // if (it == data_.end()) {
    //     return false;
    // }
    // data_.erase(it);
    // return true;
    return data_.erase(key) != 0;
}

// 获取当前键值对数量
std::size_t KVStore::size() const { return data_.size(); }