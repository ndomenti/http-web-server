

A lightweight high throughput C++20 web server, featuring epoll-driven I/O and zero-copy sendfile transfers.

## Features

- **Event-Driven Networking:** Non-blocking socket I/O via Linux `epoll` for concurrent clients.
- **Zero-Copy File Transfers:** Streams static files directly from kernel space via `sendfile()`.
- **Zero-Copy HTTP Parsing:** Parses request lines and headers in-place with `std::string_view`.

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

```bash
docker build -t web_server .
docker run --rm -p 80:80 web_server
```

### Testing

It is best to test the running server directly from your web browser at **[http://localhost](http://localhost)** so you can load the full frontend (`index.html`, `styles.css`, `script.js`) and test streaming the large PNG assets in `static/png/`.
