#pragma once

#include <string>
#include <vector>
#include <ostream>

struct HttpRequest {
    std::string_view method_;
    std::string_view url_;
    std::string_view version_;
    std::string_view headers_;

    static HttpRequest from_bytes(const std::vector<char>& bytes);
    friend std::ostream& operator<<(std::ostream& os, const HttpRequest& req);
};