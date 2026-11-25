cmake_minimum_required(VERSION 3.16)

project(MiniKVServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 公共基础库
add_library(mini_kv_core
    src/unique_fd.cpp
    src/io_utils.cpp
    src/connection_manager.cpp
    src/append_log.cpp
    src/tcp_server.cpp
)

target_include_directories(mini_kv_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)


# 主程序
add_executable(mini_kv_server src/main.cpp)

target_link_libraries(mini_kv_server PRIVATE mini_kv_core)

# 启用测试
enable_testing()

# I/O 单元测试
add_executable(test_io_utils tests/test_io_utils.cpp)
add_executable(test_append_log tests/test_append_log.cpp)
add_executable(test_dup tests/test_dup.cpp)
add_executable(test_nonblocking tests/test_nonblocking.cpp)
add_executable(test_tcp_server tests/test_tcp_server.cpp)

target_link_libraries(test_io_utils PRIVATE mini_kv_core)
target_link_libraries(test_append_log PRIVATE mini_kv_core)
target_link_libraries(test_dup PRIVATE mini_kv_core)
target_link_libraries(test_nonblocking PRIVATE mini_kv_core)
target_link_libraries(test_tcp_server PRIVATE mini_kv_core)
