import socket, time

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(("localhost", 8080))

s.sendall(b"GET /")
time.sleep(1)
s.sendall(b" HTTP/1.1\r\nHost: localhost\r\n\r\n")

res = s.recv(4096).decode()
print(res)
assert "200 OK" in res
assert "Site 1" in res
s.close()
print("FRAGMENT TEST PASSED")