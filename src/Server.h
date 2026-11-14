#pragma once

#include <memory>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <unordered_map>

#include "Connection.h"

#define EVENTS_BUFF_SIZE 1024

class Server {
public:
    Server(int port) {
        listen_fd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        if (listen_fd_ == -1) {
            throw std::runtime_error("Error creating socket");
        }

        int opt = 1;
        setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        sockaddr_in addr {
            .sin_family = AF_INET,
            .sin_port = htons(port),
            .sin_addr = {.s_addr = INADDR_ANY}
        };

        if (bind(listen_fd_, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) == -1) {
            throw std::runtime_error("Error binding socket");
        }

        if (listen(listen_fd_, SOMAXCONN) == -1) {
            throw std::runtime_error("Error listening on socket");
        }

        // std::cout << "Listening..." << std::endl;
        epoll_fd_ = epoll_create1(0);
        if (epoll_fd_ == -1) {
            throw std::runtime_error("Error creating epoll");
        }

        struct epoll_event ev {
            .events = EPOLLIN,
            .data = {.fd = listen_fd_}
        };

        if(epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_fd_, &ev) == -1) {
            throw std::runtime_error("Error adding to epoll");
        }
    }

    ~Server() {
        fd_to_conn_.clear();
        close(listen_fd_);
        close(epoll_fd_);
    }

    void run () {
        epoll_event events_buff[EVENTS_BUFF_SIZE];
        while (true) {
            int events_count = epoll_wait(epoll_fd_, events_buff, EVENTS_BUFF_SIZE, -1);
            if (events_count == -1) {
                throw std::runtime_error("Error waiting on events");
            }
            for (int i = 0; i < events_count; ++i) {
                epoll_event ev = events_buff[i];
                if (ev.data.fd == listen_fd_) {
                    handle_accept();
                } else {
                    handle_io(ev);
                }
            }
        }
    }

private:
    int listen_fd_;
    int epoll_fd_;
    std::unordered_map<int, std::unique_ptr<Connection>> fd_to_conn_;

    void handle_accept() {
        while (true) {
            sockaddr_in addr;
            socklen_t addr_len = sizeof(addr);

            int conn_fd = accept4(listen_fd_, reinterpret_cast<struct sockaddr *>(&addr), &addr_len, SOCK_NONBLOCK);
            if (conn_fd == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;
                }
                throw std::runtime_error("Error accepting connection");
            }
            // std::cout << "Accepted..." << std::endl;
            fd_to_conn_.emplace(conn_fd, std::make_unique<Connection>(conn_fd, epoll_fd_));
        }
    }

    void handle_io(const epoll_event& ev) {
        int conn_fd = ev.data.fd;
        Connection& conn = *fd_to_conn_.at(conn_fd);

        if (ev.events & EPOLLIN) {
            if (!conn.handle_read()) {
                fd_to_conn_.erase(conn_fd);
            }
        }
        else if (ev.events & EPOLLOUT) {
            if (!conn.handle_write()) {
                fd_to_conn_.erase(conn_fd);
            }
        }
        else if (ev.events & (EPOLLHUP | EPOLLERR)) {
            fd_to_conn_.erase(conn_fd);
        }
    }
};