#pragma once

#include <string>

struct HttpRequest {
    std::string_view method_;
    std::string_view url_;
    std::string_view version_;
    std::string_view headers_;

    static HttpRequest from_bytes(const std::vector<char>& bytes) {
        std::string_view raw(bytes.data(), bytes.size());

        size_t line_end = raw.find("\r\n");
        size_t headers_end = raw.find("\r\n\r\n");

        std::string_view line = raw.substr(0, line_end);
        size_t sp1 = line.find(' ');
        size_t sp2 = line.rfind(' ');

        return {
            line.substr(0, sp1),
            line.substr(sp1 + 1, sp2 - sp1 - 1),
            line.substr(sp2 + 1),
            raw.substr(line_end + 2, headers_end - line_end)
        };
    }

    friend std::ostream& operator<<(std::ostream& os, const HttpRequest& req) {
        os << req.method_ << ' ' << req.url_ << ' ' << req.version_ << '\n';

        std::string_view rest = req.headers_;
        while (!rest.empty()) {
            size_t end = rest.find("\r\n");
            os << "  " << rest.substr(0, end) << '\n';
            rest.remove_prefix(end + 2);
        }

        return os;
    }
};