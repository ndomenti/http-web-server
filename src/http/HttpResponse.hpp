#pragma once

#include <cstdint>
#include <string>
#include <optional>
#include <array>
#include <sys/socket.h>

#include "../net/ResponseFile.hpp"

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
    static HttpResponse success(ResponseFile body);
    static HttpResponse not_found();
    SendState send_to(int conn_fd);

private:
    std::string head_;
    size_t head_offset_{0};
    std::optional<ResponseFile> body_;

    static constexpr std::string_view get_mime_type(const std::string_view& extension);
};