#include "kv_store.h"
#include <cassert>
#include <iostream>

void test_set_get() {
    KVStore store;

    store.set("name", "Alice");

    auto result = store.get("name");

    assert(result.has_value());
    assert(result.value() == "Alice");
    assert(store.size() == 1);

    std::cout << "test_set_get passed\n";
}

void test_update() {
    KVStore store;

    store.set("name", "Alice");
    store.set("name", "Bob");

    auto result = store.get("name");

    assert(result.has_value());
    assert(result.value() == "Bob");
    assert(store.size() == 1);

    std::cout << "test_update passed\n";
}

void test_not_found() {
    KVStore store;

    store.set("name", "Alice");

    auto result = store.get("age");

    assert(!result.has_value());

    assert(store.size() == 1);

    std::cout << "test_not_found passed\n";
}

void test_delete() {
    KVStore store;

    store.set("name", "Alice");

    auto result = store.get("name");

    assert(result.has_value());

    assert(store.del("name"));

    assert(!store.get("name").has_value());
    assert(store.size() == 0);

    // 再次删除应该失败
    assert(!store.del("name"));

    std::cout << "test_delete passed\n";
}

void test_delete_not_found() {
    KVStore store;

    store.set("name", "Alice");

    auto result = store.get("name");

    assert(result.has_value());

    assert(!store.del("age"));

    assert(store.size() == 1);
    assert(store.get("name") == "Alice");

    std::cout << "test_delete_not_found passed\n";
}

void test_multiple_keys() {
    KVStore store;

    store.set("name", "Alice");
    store.set("age", "20");

    auto result = store.get("name");

    assert(result.has_value());
    assert(result.value() == "Alice");

    result = store.get("age");
    assert(result.has_value());
    assert(result.value() == "20");

    assert(store.size() == 2);

    std::cout << "test_multiple_keys passed\n";
}

void test_empty_value() {
    KVStore store;

    store.set("name", "");

    auto result = store.get("name");

    assert(result.has_value());
    assert(result.value() == "");

    assert(store.size() == 1);

    std::cout << "test_empty_value passed\n";
}

int main() {
    test_set_get();
    test_update();
    test_not_found();
    test_delete();
    test_delete_not_found();
    test_multiple_keys();
    test_empty_value();
}