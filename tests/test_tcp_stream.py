# import socket

# with socket.create_connection(("127.0.0.1", 8080)) as s:
#     s.sendall(b"Hello\nWorld\n")

#     result = b""

#     while result.count(b"\n") < 2:
#         data = s.recv(4096)

#         if not data:
#             break

#         result += data

#     print(result.decode())

import socket

with socket.create_connection(("127.0.0.1", 8080)) as s:
    s.settimeout(1.0)

    s.sendall(b"Hello")

    try:
        data = s.recv(4096)

        if data:
            print("FAIL: unexpected response:", data)
        else:
            print("FAIL: connection closed unexpectedly")

    except socket.timeout:
        print("PASS: incomplete message, no response")