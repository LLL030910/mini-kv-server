#include "connection_manager.h"

void ConnectionManager::add(int id) {
    if (connections_.find(id) != connections_.end())
        return;
    connections_.emplace(id, std::make_shared<Connection>(id));
}

void ConnectionManager::remove(int id) {
    auto it = connections_.find(id);
    if (it != connections_.end())
        connections_.erase(it);
}

std::shared_ptr<Connection> ConnectionManager::get(int id) const {
    auto it = connections_.find(id);
    if (it != connections_.end()) {
        return it->second;
    } else {
        return nullptr;
    }
}

std::size_t ConnectionManager::size() const { return connections_.size(); }