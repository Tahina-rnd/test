#!/usr/bin/env python3
import os
import sys

body = sys.stdin.buffer.read()
content_type = os.environ.get("CONTENT_TYPE", "text/plain")
content_length = os.environ.get("CONTENT_LENGTH", str(len(body)))
method = os.environ.get("REQUEST_METHOD", "")

sys.stdout.buffer.write(b"Status: 200 OK\r\n")
sys.stdout.buffer.write(("Content-Type: " + content_type + "\r\n").encode("latin1"))
sys.stdout.buffer.write(("X-Echo-Method: " + method + "\r\n").encode("latin1"))
sys.stdout.buffer.write(("X-Echo-Length: " + str(len(body)) + "\r\n").encode("latin1"))
sys.stdout.buffer.write(("X-Env-Content-Length: " + content_length + "\r\n\r\n").encode("latin1"))
sys.stdout.buffer.write(body)
