#include "io_utils.h"
#include "tcp_server.h"

#include <iostream>
#include <string>
#include <sys/socket.h>

void handle_client(int fd) {
    char buf[4096];

    while (true) {
        ssize_t n = recv(fd, buf, sizeof(buf), 0);

        if (n > 0) {
            // 1：打印接收到的字节数
            std::cout << "n = " << n << std::endl;

            // 2：使用 send_all 将数据原样返回
            send_all(fd, buf, static_cast<std::size_t>(n));
        } else if (n == -1) {
            // 4：处理 EINTR
            if (errno == EINTR) {
                continue;
            }
            // 5：处理其他错误
            throw std::system_error(errno, std::generic_category(), "recv");
        } else {
            std::cout << "Client disconnected\n";
            break;
        }
    }
}

int main() {
    TcpServer server(8080);
    server.start();

    while (true) {
        std::cout << "Waiting for client...\n";

        UniqueFd client = server.accept_connection();

        try {
            handle_client(client.get());
        } catch (const std::exception &e) {
            std::cerr << e.what() << '\n';
        }
    }
}