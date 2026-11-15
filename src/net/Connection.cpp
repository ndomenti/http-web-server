#include "Connection.hpp"

Connection::Connection(int conn_fd, int epoll_fd) :
conn_fd_(conn_fd), epoll_fd_(epoll_fd) {
    epoll_event read_ev {
        .events = EPOLLIN | EPOLLET,
        .data = {.fd = conn_fd_}
    };
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, conn_fd_, &read_ev);
}

Connection::~Connection() {
    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, conn_fd_, nullptr);
    close(conn_fd_);
    // std::cout << "Disconnected..." << std::endl;
}

bool Connection::handle_write() {
    if (!incoming_.empty()) {
        HttpRequest req = HttpRequest::from_bytes(incoming_);
        // std::cout << req << std::endl;
        std::optional<ResponseFile> file = ResponseFile::open_read(resolve_url(req.url_));
        incoming_.clear();

        response_ = file
        ? HttpResponse::success(std::move(file.value()))
        : HttpResponse::not_found();
    }

    if (response_) {
        SendState state = response_->send_to(conn_fd_);
        switch (state) {
            case SendState::InProgress:
                return true;
            case SendState::Error:
                return false;
            case SendState::Complete: {
                response_ = std::nullopt;
                set_read_mode();
                return true;
            }
        }
    }

    return true;
}

bool Connection::handle_read() {
    uint8_t recv_buff[RECV_BUFF_SIZE];
    while (true) {
        int bytes_read = recv(conn_fd_, recv_buff, RECV_BUFF_SIZE, 0);
        if (bytes_read == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            return false;
        }
        if (bytes_read == 0) {
            return false;
        }
        // std::cout << "Read " << bytes_read << " bytes" << std::endl;
        incoming_.insert(incoming_.end(), recv_buff, recv_buff + bytes_read);
    }
    if (is_request_complete()) {
        set_write_mode();
    }
    return true;
}

bool Connection::is_request_complete() const {
    size_t n = incoming_.size();
    if (n < 4) {
        return false;
    }
    return (incoming_[n-4] == '\r' &&
            incoming_[n-3] == '\n' &&
            incoming_[n-2] == '\r' &&
            incoming_[n-1] == '\n');
}

std::string_view Connection::resolve_url(std::string_view url) {
    return (url == "/") ? "index.html" : url.substr(1);
}

void Connection::set_write_mode() const {
    epoll_event write_ev {
        .events = EPOLLOUT | EPOLLET,
        .data = {.fd = conn_fd_}
    };
    epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, conn_fd_, &write_ev);
}

void Connection::set_read_mode() const {
    epoll_event read_ev {
        .events = EPOLLIN | EPOLLET,
        .data = {.fd = conn_fd_}
    };
    epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, conn_fd_, &read_ev);
}

