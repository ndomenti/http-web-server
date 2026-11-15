#pragma once

#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <utility>
#include <vector>

#include "ResponseFile.hpp"
#include "../http/HttpRequest.hpp"
#include "../http/HttpResponse.hpp"

#define RECV_BUFF_SIZE 1024

class Connection {
public:
    Connection(int conn_fd, int epoll_fd);
    ~Connection();

    bool handle_write();
    bool handle_read();

private:
    int conn_fd_{-1};
    int epoll_fd_{-1};
    std::vector<char> incoming_;
    std::optional<HttpResponse> response_;

    [[nodiscard]] bool is_request_complete() const;

    static std::string_view resolve_url(std::string_view url);
    void set_write_mode() const;
    void set_read_mode() const;
};
