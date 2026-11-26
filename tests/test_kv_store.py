import socket

def send_command(command):
    with socket.create_connection(
        ("127.0.0.1", 8080), timeout=3
    ) as s:

        s.sendall((command + "\n").encode())

        with s.makefile("rb") as reader:
            return reader.readline()


# 客户端 A：写入数据，随后断开连接
result = send_command("SET session Alice")
assert result == b"OK\n"

print("Client A: SET session Alice -> OK")


# 客户端 B：重新建立连接，读取数据
result = send_command("GET session")
assert result == b"Alice\n"

print("Client B: GET session -> Alice")

print("PASS: data shared across clients")