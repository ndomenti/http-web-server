#pragma once

#include <string>
#include <optional>
#include <fcntl.h>
#include <filesystem>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <unistd.h>


class ResponseFile {
public:
    static std::optional<ResponseFile> open_read(const std::string_view file_path);

    explicit ResponseFile (const int fd, size_t size, std::string extension);

    ResponseFile(ResponseFile& other) = delete;
    ResponseFile& operator=(ResponseFile& other) = delete;

    ResponseFile(ResponseFile&& other) noexcept;
    ResponseFile& operator=(ResponseFile&& other) noexcept;

    ~ResponseFile();

    ssize_t send_to(int conn_fd);
    size_t get_size() const;
    const std::string& get_extension();

private:
    int fd_{-1};
    off_t off_{0};
    size_t size_{0};
    std::string extension_;
};
