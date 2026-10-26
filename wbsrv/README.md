*This project has been created as part of the 42 curriculum by jrabenah, tarandri.*

# webserv

A small HTTP/1.1 web server written in C++98, built around a single
non-blocking `poll()` loop. It reads an nginx-inspired configuration file,
serves static websites, accepts uploads and file deletions, supports several
ports and virtual hosts, and keeps connections alive.

> **Project status:** the network core, the configuration parser, the static
> HTTP server and POST/DELETE/upload (Sprints 1–4) are implemented and tested.
> The CGI (Sprint 5) and the final hardening/demo pass (Sprint 6) remain.
> See [Current status](#current-status).

---

## Description

`webserv` is a from-scratch HTTP server. It does not use an external HTTP
library and does not launch another web server. The whole server is driven by
one event loop:

```text
poll() → accept() a client → recv() → parse HTTP → build response → send()
```

Main features already available:

- non-blocking sockets and a single `poll()` for all network I/O;
- nginx-like configuration with `http`, `server` and `location` blocks;
- static file serving with MIME types;
- `index` files, directory redirection and optional autoindex;
- HTTP/1.0 and HTTP/1.1 requests;
- keep-alive, request pipelining and chunked request bodies;
- `POST` uploads to `upload_store` (`201 Created` + `Location`);
- `DELETE` of regular files (`204 No Content`, `404`, `403`);
- custom and default error pages;
- explicit method dispatch (`GET`, `POST`, `DELETE`) with `405` + `Allow`;
- several listening ports and virtual hosts based on the `Host` header;
- 8 KB request-line/header limits, body-size limits and path-traversal
  protection;
- an inactivity timeout so a request can never stay suspended forever.

The project is designed for Linux. It compiles with `-std=c++98 -Wall -Wextra
-Werror`.

---

## Instructions

### Requirements

- Linux
- `make`
- a C++98-compatible compiler (`g++` or `clang++`)
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
./webserv configs/demo_multi.conf  # three ports: 8080, 8081, 8082
./webserv configs/demo_vhosts.conf # two virtual hosts on port 8080
```

Then open `http://localhost:8080/` in a browser or use `curl`. The home page
links to `/upload_demo.html`, a browser test console covering Sprints 1 to 4
(GET, parallel requests, keep-alive, redirect, 403/404/413/414/431, traversal,
MIME, POST/upload, DELETE and 405).

```bash
curl -i http://localhost:8080/
curl -i http://localhost:8080/index.html
curl -i -H "Host: alpha.local" http://localhost:8080/   # vhost demo

# Upload the raw request body to www/site1/upload/
curl -i -X POST --data-binary @README.md http://localhost:8080/upload

# Delete a regular file
curl -i -X DELETE http://localhost:8080/upload/README.md
```

Stop the server with `Ctrl+C`.

### Tests

```bash
./test/config_tests.sh                    # configuration: 105 tests
make test-tokenizer && ./test_tokenizer   # tokenizer: 20 tests
make test-config && ./test_config configs/default.conf
make test-http && ./test_http             # HTTP: 46 tests
./test/sprint3_tests.sh                   # static/routing integration
./test/sprint4_tests.sh                   # POST/DELETE/upload: 54 tests

valgrind --leak-check=full ./test_http    # memory check
```

The server itself can be stress-tested with:

```bash
ab -n 2000 -c 100 http://localhost:8080/
```

### Configuration example

```nginx
http {
    server {
        listen 8080;
        server_name localhost;
        root www/site1;
        index index.html;
        client_max_body_size 1M;

        location / {
            allow_methods GET;
            root www/site1;
            autoindex off;
        }

        location /upload {
            allow_methods GET POST DELETE;
            root www/site1;
            upload_store www/site1/upload;
        }

        location /redirect {
            return 301 http://localhost:8080/;
        }
    }
}
```

Supported directives: `listen`, `server_name`, `root`, `index`,
`client_max_body_size`, `error_page`, `allow_methods`, `return`, `autoindex`,
`upload_store`, `cgi_ext`, `cgi_pass`.

If the configuration is invalid, the server prints a message with the line
number and exits before opening any socket:

```text
Config error: config: line 3: invalid port '99999'
```

---

## Project structure

```text
includes/
├── config/    configuration structures and parser
├── http/      Request, Response, Mime
├── network/   Socket, Connection, Server
└── utils/     helpers
srcs/          implementation of the above
configs/       example configuration files
www/           static websites served by the demos
test/          shell, Python and C++ tests
```

The documentation is split by purpose:

| File | Purpose |
|---|---|
| `STEP.md` | roadmap, sprint status and next tasks (French) |
| `EXPLAIN.md` | how the code works, class by class (French) |
| `NOTES.md` | subject constraints and compliance checklist (French) |
| `DECISIONS.md` | technical decisions and their rationale (French) |
| `sujet.fr.md` | official 42 subject, reference only (French) |

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
| CGI | 🕓 planned |
| Full stress/valgrind campaign | ⚠️ partial |

Known limitations:

- `HEAD` is not implemented; with the provided configurations it returns `405`
  (the subject only requires GET, POST and DELETE).
- Responses are always written as `HTTP/1.1`, even for an HTTP/1.0 client.
- Uploads are stored as raw request bodies: `multipart/form-data` boundaries
  are not parsed. Use `--data-binary` or another raw body format.
- The CGI script and its demo directory are still to be added (Sprint 5).

---

## Resources

- RFC 7230 — HTTP/1.1 Message Syntax and Routing
- RFC 7231 — HTTP/1.1 Semantics and Content
- RFC 7232 — HTTP/1.1 Conditional Requests
- RFC 3875 — The Common Gateway Interface (CGI) Version 1.1
- nginx documentation, especially the `server` and `location` directives:
  <https://nginx.org/en/docs/http/ngx_http_core_module.html>
- 42 `webserv` subject (`sujet.fr.md` in this repository)
- Linux man pages: `poll(2)`, `socket(2)`, `bind(2)`, `listen(2)`, `recv(2)`,
  `send(2)`, `fcntl(2)`, `getaddrinfo(3)`, `waitpid(2)`

### AI usage

AI was used as a review and documentation assistant:

- re-reading the subject and checking eliminatory constraints;
- proposing edge cases and test scenarios (malformed requests, size limits,
  path traversal, chunked bodies);
- helping to write and structure the documentation;
- suggesting code fixes, which were then reviewed, compiled and validated with
  the test suite.

AI was not used to copy an existing HTTP server. The architecture, the code
and the final technical choices remain the team's responsibility.
