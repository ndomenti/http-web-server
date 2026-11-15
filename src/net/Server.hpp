#pragma once

#include <memory>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <unordered_map>

#include "Connection.hpp"

#define EVENTS_BUFF_SIZE 1024

class Server {
public:
    Server(int port);
    ~Server();

    void run();

private:
    int listen_fd_;
    int epoll_fd_;
    std::unordered_map<int, std::unique_ptr<Connection>> fd_to_conn_;

    void handle_accept();
    void handle_io(const epoll_event& ev);
};