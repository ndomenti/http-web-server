#include "ResponseFile.hpp"

std::optional<ResponseFile> ResponseFile::open_read(const std::string_view file_path) {
    char path_buff[PATH_MAX];
    if (file_path.size() > sizeof(path_buff)) {
        return std::nullopt;
    }

    memcpy(path_buff, file_path.data(), file_path.size());
    path_buff[file_path.size()] = '\0';

    int file_fd = open(path_buff, O_RDONLY);
    if (file_fd == -1) {
        return std::nullopt;
    }

    struct stat stat_buf{};
    if (fstat(file_fd, &stat_buf) == -1) {
        close(file_fd);
        return std::nullopt;
    }

    size_t dot = file_path.rfind('.');
    std::string_view extension = file_path.substr(dot);

    return ResponseFile(file_fd, stat_buf.st_size, std::string(extension));
}

ResponseFile::ResponseFile (const int fd, size_t size, std::string extension)
: fd_(fd), size_(size), extension_(std::move(extension)) {}

ResponseFile::ResponseFile(ResponseFile&& other) noexcept {
    fd_ = other.fd_;
    off_ = other.off_;
    size_ = other.size_;
    extension_ = other.extension_;
    other.fd_ = -1;
};

ResponseFile& ResponseFile::operator=(ResponseFile&& other) noexcept {
    if (this != &other) {
        if (fd_ != -1) {
            close(fd_);
        }
        fd_ = other.fd_;
        off_ = other.off_;
        size_ = other.size_;
        extension_ = other.extension_;
        other.fd_ = -1;
    }
    return *this;
};

ResponseFile::~ResponseFile() {
    if (fd_ != - 1) {
        close(fd_);
    }
}

ssize_t ResponseFile::send_to(int conn_fd) {
    return sendfile(conn_fd, fd_, &off_, size_ - off_);
}

size_t ResponseFile::get_size() const {
    return size_;
}

const std::string& ResponseFile::get_extension() {
    return extension_;
}