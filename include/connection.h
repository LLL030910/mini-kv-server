#ifndef CONNECTION_H
#define CONNECTION_H

#include <iostream>

class Connection {
public:
    explicit Connection(int id) : id_(id) {
        std::cout << "Connection " << id_ << " created\n";
    }

    ~Connection() { std::cout << "Connection " << id_ << " destroyed\n"; }

    int id() const { return id_; }

    void send() const {
        std::cout << "Connection " << id_ << " sending data\n";
    }

private:
    int id_;
};

#endif // CONNECTION_H