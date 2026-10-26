*This project has been created as part of the 42 curriculum by jrabenah, tarandri.*

# webserv

A small HTTP/1.1 web server written in C++98, built around a single
non-blocking `poll()` loop. It reads an nginx-inspired configuration file,
serves static websites, accepts uploads and file deletions, executes CGI/1.1
scripts, supports several ports and virtual hosts, and keeps connections alive.

> **Project status:** the network core, the configuration parser, the static
> HTTP server, POST/DELETE/upload and CGI/1.1 execution (Sprints 1–5) are
> implemented and tested (292 automated checks passing).
> See [Current status](#current-status).

---

## Description

`webserv` is a from-scratch HTTP server. It does not use an external HTTP
library and does not launch another web server. The whole server is driven by
one event loop:

```text
poll() → accept() a client → recv() → parse HTTP → CGI pipes / build response → send()
```

Main features available:

- non-blocking sockets and a single `poll()` for all network and CGI pipe I/O;
- nginx-like configuration with `http`, `server` and `location` blocks;
- static file serving with MIME types;
- `index` files, directory redirection and optional autoindex;
- HTTP/1.0 and HTTP/1.1 requests;
- keep-alive, request pipelining and chunked request bodies;
- `POST` uploads to `upload_store` (`201 Created` + `Location`);
- `DELETE` of regular files (`204 No Content`, `404`, `403`);
- CGI/1.1 execution (`fork` + `chdir` + `dup2` + `execve`) with non-blocking
  pipes registered in `poll()`, full CGI environment variables, un-chunked
  request body on `stdin`, and a 10-second timeout (`504 Gateway Timeout`)
  without zombie processes;
- custom and default error pages;
- explicit method dispatch (`GET`, `POST`, `DELETE`) with `405` + `Allow`;
- several listening ports and virtual hosts based on the `Host` header;
- 8 KB request-line/header limits, body-size limits and path-traversal
  protection;
- a 60-second inactivity timeout so a request can never stay suspended forever.

The project is designed for Linux. It compiles with `-std=c++98 -Wall -Wextra
-Werror`.

---

## Instructions

### Requirements

- Linux
- `make`
- a C++98-compatible compiler (`g++` or `clang++`)
- Python 3 (`/usr/bin/python3`) for CGI demo scripts
- optional: `curl`, `valgrind`, `apache2-utils` (`ab`) for testing

### Build

```bash
make          # build ./webserv
make re       # clean rebuild
make clean    # remove object files
make fclean   # remove object files and the binary
```

### Run

```bash
./webserv                          # uses configs/default.conf
./webserv configs/default.conf     # explicit configuration
./webserv configs/cgi.conf         # dedicated CGI demo configuration
./webserv configs/demo_multi.conf  # three ports: 8080, 8081, 8082
./webserv configs/demo_vhosts.conf # two virtual hosts on port 8080
```

Then open `http://localhost:8080/` in a browser or use `curl`. The home page
links to `/upload_demo.html`, a browser test console covering Sprints 1 to 5
(GET, parallel requests, keep-alive, redirect, 403/404/413/414/431, traversal,
MIME, POST/upload, DELETE, 405 and CGI/1.1).

```bash
curl -i http://localhost:8080/
curl -i http://localhost:8080/index.html
curl -i -H "Host: alpha.local" http://localhost:8080/   # vhost demo

# Upload the raw request body to www/site1/upload/
curl -i -X POST --data-binary @README.md http://localhost:8080/upload

# Delete a regular file
curl -i -X DELETE http://localhost:8080/upload/README.md

# Execute a Python CGI script (GET and POST)
curl -i "http://localhost:8080/cgi-bin/hello.py?name=42"
curl -i -X POST --data-binary "Hello CGI" http://localhost:8080/cgi-bin/echo.py
```

Stop the server with `Ctrl+C`.

### Tests

```bash
./test/config_tests.sh                    # configuration: 105 tests
make test-tokenizer && ./test_tokenizer   # tokenizer: 20 tests
make test-config && ./test_config configs/default.conf
make test-http && ./test_http             # HTTP: 46 tests
./test/sprint3_tests.sh                   # static/routing: 27 tests
./test/sprint4_tests.sh                   # POST/DELETE/upload: 54 tests
./test/sprint5_tests.sh                   # CGI/1.1 & timeouts: 40 tests

valgrind --leak-check=full ./test_http    # memory check
```

The server itself can be stress-tested with:

```bash
ab -n 2000 -c 100 http://localhost:8080/
```

---

## Project structure

```text
includes/
├── cgi/       CgiHandler (non-blocking CGI/1.1 execution)
├── config/    configuration structures and parser
├── http/      Request, Response, Mime
├── network/   Socket, Connection, Server
└── utils/     helpers
srcs/          implementation of the above
configs/       example configuration files
www/           static websites and CGI scripts served by the demos
test/          shell, Python and C++ tests
```

---

## Current status

| Area | Status |
|---|---|
| Non-blocking network, single `poll()` | ✅ |
| Configuration parser and validation | ✅ |
| Static files, MIME, index, autoindex | ✅ |
| Keep-alive, pipelining, chunked input | ✅ |
| Multi-port and virtual hosts | ✅ |
| Default error pages | ✅ |
| Custom error pages | ✅ |
| POST / DELETE / upload | ✅ |
| CGI/1.1 (Python, GET/POST, 10s timeout) | ✅ |
| Full stress/valgrind campaign | ✅ |
