#include "command_handler.h"
#include "kv_store.h"
#include <cassert>
#include <iostream>

void test_ping() {
    KVStore store;

    assert(handle_command(store, "PING") == "PONG\n");

    std::cout << "test_ping passed\n";
}

void test_set_get() {
    KVStore store;

    assert(handle_command(store, "SET name Alice") == "OK\n");

    assert(handle_command(store, "GET name") == "Alice\n");

    assert(store.size() == 1);

    std::cout << "test_set_get passed\n";
}

void test_update() {
    KVStore store;

    assert(handle_command(store, "SET name Alice") == "OK\n");
    assert(handle_command(store, "SET name Mike") == "OK\n");
    assert(handle_command(store, "GET name") == "Mike\n");

    assert(store.size() == 1);

    std::cout << "test_update passed\n";
}

void test_not_found() {
    KVStore store;

    assert(handle_command(store, "SET name Alice") == "OK\n");

    assert(handle_command(store, "GET age") == "NOT_FOUND\n");

    assert(store.size() == 1);

    std::cout << "test_not_found passed\n";
}

void test_delete() {
    KVStore store;

    assert(handle_command(store, "SET name Alice") == "OK\n");

    assert(handle_command(store, "DEL name") == "1\n");

    assert(store.size() == 0);

    std::cout << "test_delete passed\n";
}

void test_delete_not_found() {
    KVStore store;

    assert(handle_command(store, "SET name Alice") == "OK\n");

    assert(handle_command(store, "DEL age") == "0\n");

    assert(store.size() == 1);

    std::cout << "test_delete_not_found passed\n";
}

void test_missing_args() {
    KVStore store;

    assert(handle_command(store, "SET") == "ERR\n");
    assert(handle_command(store, "SET name") == "ERR\n");
    assert(handle_command(store, "GET") == "ERR\n");
    assert(handle_command(store, "DEL") == "ERR\n");

    assert(store.size() == 0);

    std::cout << "test_missing_args passed\n";
}

void test_extra_args() {
    KVStore store;

    assert(handle_command(store, "PING abc") == "ERR\n");

    assert(handle_command(store, "SET name Alice 13") == "ERR\n");

    assert(handle_command(store, "GET name age") == "ERR\n");

    assert(handle_command(store, "DEL name age") == "ERR\n");

    assert(store.size() == 0);

    std::cout << "test_extra_args passed\n";
}

void test_invalid_command() {
    KVStore store;

    assert(handle_command(store, "SAT name Alice") == "ERR\n");

    assert(store.size() == 0);

    std::cout << "test_invalid_command passed\n";
}

void test_empty_command() {
    KVStore store;

    assert(handle_command(store, "") == "ERR\n");

    assert(store.size() == 0);

    std::cout << "test_empty_command passed\n";
}

void test_invalid_set() {
    KVStore store;

    store.set("name", "Alice");

    auto result = handle_command(store, "SET name Bob extra");

    assert(result == "ERR\n");
    assert(store.get("name") == "Alice");
    assert(store.size() == 1);

    std::cout << "test_invalid_set passed\n";
}

void test_invalid_del() {
    KVStore store;

    store.set("name", "Alice");

    auto result = handle_command(store, "DEL name extra");

    assert(result == "ERR\n");
    assert(store.get("name") == "Alice");
    assert(store.size() == 1);

    std::cout << "test_invalid_del passed\n";
}

int main() {
    test_ping();
    test_set_get();
    test_update();
    test_not_found();
    test_delete();
    test_delete_not_found();
    test_missing_args();
    test_extra_args();
    test_invalid_command();
    test_empty_command();
    test_invalid_set();
    test_invalid_del();
}
