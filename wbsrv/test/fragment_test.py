import socket, time

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(("localhost", 8080))

s.sendall(b"GET /")
time.sleep(2)
s.sendall(b" HTTP/1.1\r\n\r\n")

print(s.recv(4096).decode())
s.close()