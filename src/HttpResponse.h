#pragma once

#include <cstdint>
#include <string>
#include <optional>
#include <array>
#include <sys/socket.h>

#include "ResponseFile.h"


enum class SendState {
    InProgress,
    Complete,
    Error
};

enum class HttpStatus : uint16_t {
    Ok = 200,
    NotFound = 404,
};


class HttpResponse {
public:
    static HttpResponse success(ResponseFile body) {
        HttpResponse res;
        res.head_.reserve(128);
        res.head_.append("HTTP/1.1 200 OK\r\nContent-Length: ");
        res.head_.append(std::to_string(body.get_size()));
        res.head_.append("\r\nContent-Type: ");
        res.head_.append(get_mime_type(body.get_extension()));
        res.head_.append("\r\n\r\n");
        res.body_ = std::move(body);
        return res;
    }

    static HttpResponse not_found() {
        HttpResponse res;
        res.head_ = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
        return res;
    }

    SendState send_to(int conn_fd) {
        while (head_offset_ < head_.size()) {
            ssize_t bytes_sent = send(
                conn_fd,
                head_.data() + head_offset_,
                head_.size() - head_offset_,
                0);
            if (bytes_sent == -1) {
                return (errno == EAGAIN || errno == EWOULDBLOCK)
                    ? SendState::InProgress
                    : SendState::Error;
            }
            if (bytes_sent == 0) {
                return SendState::InProgress;
            }
            head_offset_ += bytes_sent;
            // std::cout << "[HEAD] Sent " << bytes_sent << " bytes" << std::endl;
        }

        while (body_) {
            ssize_t bytes_sent = body_->send_to(conn_fd);
            if (bytes_sent == -1) {
                return (errno == EAGAIN || errno == EWOULDBLOCK)
                    ? SendState::InProgress
                    : SendState::Error;
            }
            if (bytes_sent == 0) {
                body_ = std::nullopt;
                break;
            }
            // std::cout << "[BODY] Sent " << bytes_sent << " bytes" << std::endl;
        }

        return SendState::Complete;
    }

private:
    std::string head_;
    size_t head_offset_{0};
    std::optional<ResponseFile> body_;

    static constexpr std::string_view get_mime_type(const std::string_view& extension) {
        constexpr std::array<std::pair<std::string_view, std::string_view>, 9> mines {{
                {".html", "text/html"},
                {".css",  "text/css"},
                {".js",   "text/javascript"},
                {".json", "application/json"},
                {".png",  "image/png"},
                {".jpg",  "image/jpeg"},
                {".jpeg", "image/jpeg"},
                {".svg",  "image/svg+xml"},
                {".ico",  "image/x-icon"}
        }};
        for (const auto& [ext, mine] : mines) {
            if (ext == extension) {
                return mine;
            }
        }
        return "application/octet-stream";
    }
};