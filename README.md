*This project has been created as part of the 42 curriculum by bgrosjea, amirloup, achatzit.*

# Webserv

> This is when you finally understand why URLs start with HTTP.

## Description

**Webserv** is an HTTP/1.1 server written in C++98, built by a team of three. It serves a static website, handles file uploads, downloads and deletions, and runs Python CGI scripts, all from a single process that never blocks.

The goal of the project is to understand what happens behind a URL: sockets, the HTTP request and response format, status codes, and how one server can talk to many clients at once without threads.

### Features

- **Non-blocking I/O with a single `epoll`**: one `epoll_wait` loop watches the listening sockets and every client socket, for reading and writing at the same time. No `recv` or `send` happens without `epoll` reporting the socket as ready.
- **Several ports**: the server listens on every port listed in the configuration file (`4343` and `4444` by default).
- **Methods**: `GET`, `POST` and `DELETE`, allowed per route in the configuration file. A method not allowed on a route gets `403`.
- **Static website**: HTML pages served from `site/`, and directory listing when the URL points to a directory.
- **Uploads, downloads and deletions**: files are uploaded through a Python CGI (`multipart/form-data`) into `site/downloads/`, listed on `/download`, and removed with a `DELETE` request.
- **CGI**: scripts in `cgi-bin/` run in a child process (`fork` + `execve`). The request is sent to the script through a pipe, its output is read back through another pipe, and it receives `REQUEST_METHOD`, `PATH_INFO` and the multipart `BOUNDARY` in its environment. A script that fails or takes too long returns `500`.
- **HTTP redirection**: routes declared as `redirection` answer `302` with a `Location` header.
- **Status codes and error pages**: `200`, `202`, `302`, `400`, `403`, `404`, `408`, `413`, `415`, `500`… with one error page per code, configurable in the configuration file.
- **Body size limit**: a request body larger than `bodysize` is rejected with `413`.
- **Bonus: cookies and sessions**: logging in on `/login` creates a session. The server sends a `session_id` cookie (`HttpOnly`), recognises the user on the next requests, shows who is logged in, and `/logout` ends the session.

## Instructions

### Requirements

- Linux (the server uses `epoll`), `c++`, `make`
- `python3` for the CGI scripts

### Build

```bash
git clone https://github.com/Hellisabd/Webserv.git
cd Webserv
make          # builds ./webserv with -Wall -Wextra -Werror -std=c++98
```

Other rules: `make fclean`, `make re`, and `make val` to run the server under Valgrind.

### Run

Run the server from the root of the repository (paths in the configuration file are relative to it), with a configuration file as its only argument:

```bash
mkdir -p site/downloads          # upload folder, not versioned when empty
./webserv fichierConfig/config
```

Then open <http://127.0.0.1:4343> (or port `4444`) in a browser. Stop the server with `Ctrl-C`.

### Testing with curl

```bash
curl -i http://127.0.0.1:4343/                   # 200, home page
curl -i http://127.0.0.1:4343/nope               # 404, error page
curl -i http://127.0.0.1:4343/redirect           # 302, redirection
curl -i -X DELETE http://127.0.0.1:4343/         # 403, method not allowed on this route
curl -i -F "file=@notes.txt" http://127.0.0.1:4343/cgi-bin/upload.py      # upload through the CGI
curl -i -X DELETE "http://127.0.0.1:4343/delete?filename=notes.txt"       # delete the uploaded file
head -c 20000 /dev/zero | curl -i -X POST --data-binary @- http://127.0.0.1:4343/upload   # 413, body too large
```

### Configuration file

The format is inspired by NGINX's `server` block:

```
port 4343 4444               # ports to listen on
host 127.0.0.1               # interface
bodysize 10000               # maximum body size, in bytes
server_name  bonjour lol

location /upload {           # route
	Method GET POST          # accepted methods
	./site/upload.html       # file served for this route
}

redirection /redirect {      # 302 to another URL
	https://example.com
}

error_pages {                # one page per status code
	404 ./site/404.html
	413 ./site/413.html
}
```

## How it works

1. **Configuration** (`Data`): the file is read at startup: ports, host, body size, routes with their methods, redirections and error pages.
2. **Sockets** (`ServerSocket`): one socket per port, with `SO_REUSEADDR`, then `bind` and `listen`.
3. **Event loop** (`Epoll`): the listening sockets and the clients are registered in a single `epoll` instance. A new connection is accepted and added to it. A client is watched for reading until its request is complete, then switched to writing to send the response, then back to reading.
4. **Parsing** (`HttpRequest`): the request line, the headers and the body are parsed and checked (method, URL, `Content-Length`). A malformed request gets `400`.
5. **Routing**: redirection, directory listing, CGI (`/cgi-bin/…`), deletion, or the file of the matching route. Unknown routes get `404`.
6. **Response** (`Response`): the status line and headers (`Content-Type`, `Content-Length`, `Set-Cookie`) are built, followed by the page or the matching error page.

### Technical choices

- **Event-driven, with no threads**: a single loop handles every connection. A slow client never blocks the others, because the server only reads or writes when `epoll` says the socket is ready.
- **Per-client state**: each connection keeps its own state (request received so far, expected body size, response being sent, CGI in progress, session). This lets a request arrive or leave over several loop turns.
- **CGI without blocking**: the server does not wait for the script with a blocking `waitpid`. It checks it with `WNOHANG` on each turn, and kills it (`SIGTERM`) after a timeout.
- **Clean shutdown**: `Ctrl-C` sets a flag that ends the loop, and the sockets are closed in destructors.

### Limitations

- Chunked requests (`Transfer-Encoding: chunked`) are not supported; bodies must have a `Content-Length`.
- Only Python CGI scripts are supported.
- The upload folder (`site/downloads/`) and the directory listing are fixed in the code, not set in the configuration file.

## Project structure

```
.
├── src/
│   ├── main.cpp            # configuration, sockets, then the epoll loop
│   ├── class/
│   │   ├── Data            # configuration file parsing
│   │   ├── ServerSocket    # listening sockets
│   │   ├── Epoll           # event loop, reading requests, sending responses
│   │   ├── HttpRequest     # HTTP request parsing
│   │   ├── Response        # status line, headers, error pages
│   │   ├── cgi             # CGI execution
│   │   └── Client          # logged-in users (sessions)
│   ├── uploads.cpp         # upload list and download page
│   ├── delete.cpp          # DELETE on uploaded files
│   ├── dir.cpp             # directory listing
│   └── login.cpp           # login and session IDs
├── includes/webserv.hpp    # shared includes and per-client state
├── fichierConfig/config    # default configuration file
├── cgi-bin/                # Python CGI scripts (upload, add file)
└── site/                   # website and error pages
```

## Resources

- [RFC 9112 — HTTP/1.1](https://www.rfc-editor.org/rfc/rfc9112) and [RFC 9110 — HTTP Semantics](https://www.rfc-editor.org/rfc/rfc9110): message format, methods and status codes.
- [RFC 3875 — The Common Gateway Interface](https://www.rfc-editor.org/rfc/rfc3875): the CGI environment variables and the server-script exchange.
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/): sockets, `bind`, `listen`, `accept`, `send`, `recv`.
- [MDN — HTTP](https://developer.mozilla.org/en-US/docs/Web/HTTP): headers, cookies and status codes.
- [NGINX documentation](https://nginx.org/en/docs/): the reference for the configuration file and for comparing behaviour.
- `man 7 epoll`, `man 2 epoll_ctl`, `man 2 socket`, `man 2 accept`, `man 2 fork`, `man 2 execve`.

### Use of AI

The server itself (sockets, event loop, HTTP parsing, CGI, sessions) was written by the three of us, without AI-generated code. An AI assistant (Claude) was used to write this README from the subject and the source code; we reviewed its content.
