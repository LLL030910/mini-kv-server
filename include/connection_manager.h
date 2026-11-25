#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include "connection.h"
#include <memory>
#include <unordered_map>

class ConnectionManager {
public:
    void add(int id);

    void remove(int id);

    std::shared_ptr<Connection> get(int id) const;

    std::size_t size() const;

private:
    std::unordered_map<int, std::shared_ptr<Connection>> connections_;
};

#endif