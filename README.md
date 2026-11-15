# web_server

A lightweight C++20 web server built for high throughput, featuring epoll-driven I/O and zero-copy sendfile transfers.

## Features

- **Event-Driven Networking:** Non-blocking socket I/O powered by Linux `epoll` to handle concurrent connections efficiently.
- **Zero-Copy File Transfers:** Streams static assets directly from kernel space to the client socket via `sendfile()`, avoiding user-space buffer copies even on large files.
- **Zero-Copy HTTP Parsing:** Parses request lines and headers in-place using `std::string_view` slices over the connection read buffer.
- **Non-Blocking Send Pipeline:** Tracks partial `send()` and `sendfile()` progress across `EAGAIN` / `EWOULDBLOCK` events cleanly.

## Project Structure

```text
├── CMakeLists.txt
├── Dockerfile
├── src/
│   ├── main.cpp
│   ├── http/
│   │   ├── HttpRequest.{hpp,cpp}    # Zero-copy HTTP request parser
│   │   └── HttpResponse.{hpp,cpp}   # Response headers, MIME types, and send state
│   └── net/
│       ├── Server.{hpp,cpp}         # Listening socket and epoll event loop
│       ├── Connection.{hpp,cpp}     # Per-client connection lifecycle and buffers
│       └── ResponseFile.{hpp,cpp}   # RAII file wrapper for sendfile()
└── static/                          # Document root served at /
```

## Quick Start

### Docker (Recommended)

Builds the Release binary inside an Alpine builder stage and runs it with `/app/static` as the working directory on port `80`:

```bash
docker build -t web_server .
docker run --rm -p 80:80 web_server
```

### Testing

It is best to test the running server directly from your web browser at **[http://localhost](http://localhost)** so you can load the full frontend (`index.html`, `styles.css`, `script.js`) and test streaming the large PNG assets in `static/png/`.
