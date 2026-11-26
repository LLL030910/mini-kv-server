#include "buffer.h"
#include "command_handler.h"
#include "io_utils.h"
#include "kv_store.h"
#include "tcp_server.h"
#include <iostream>
#include <string>
#include <sys/socket.h>

void handle_client(int fd, KVStore &store) {
    Buffer input_buffer;
    char buf[4096];

    while (true) {
        ssize_t n = recv(fd, buf, sizeof(buf), 0);

        if (n > 0) {
            // 1：将本次接收的数据追加到 Buffer
            input_buffer.append(buf, static_cast<std::size_t>(n));

            // 2：循环提取完整消息
            while (true) {
                // 3：调用 retrieve_line()
                auto message = input_buffer.retrieve_line();

                // 4：如果没有完整消息，结束内层循环
                if (!message.has_value()) {
                    break;
                }

                // 5：构造响应字符串
                // std::string response = "RECEIVED: " + *message + "\n";
                std::string response = handle_command(store, *message);

                // 6：使用 send_all() 返回响应
                send_all(fd, response.data(), response.size());
            }

        } else if (n == -1) {
            if (errno == EINTR) {
                continue;
            }
            throw std::system_error(errno, std::generic_category(), "recv");
        } else {
            std::cout << "Client disconnected\n";
            break;
        }
    }
}

int main() {
    KVStore store;

    TcpServer server(8080);
    server.start();

    while (true) {
        std::cout << "Waiting for client...\n";

        UniqueFd client = server.accept_connection();

        try {
            handle_client(client.get(), store);
        } catch (const std::exception &e) {
            std::cerr << e.what() << '\n';
        }
    }
}