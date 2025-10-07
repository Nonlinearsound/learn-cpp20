#include "Fd.h"
#include <cerrno>       // errno
#include <system_error> // std::system_error
#include <iostream>

Fd::Fd(int f) noexcept : fd(f) {
#ifdef DEBUG_FD
    log("constructed");
#endif
}

Fd::~Fd() noexcept {
#ifdef DEBUG_FD
    log("destructed");
#endif
    if (fd >= 0)
        ::close(fd);
}

Fd::Fd(Fd&& other) noexcept
    : fd(std::exchange(other.fd, -1)) {
#ifdef DEBUG_FD
    log("moved (ctor)");
#endif
}

Fd& Fd::operator=(Fd&& other) noexcept {
    if (this != &other) {
#ifdef DEBUG_FD
        log("moved (assign)");
#endif
        reset();
        fd = std::exchange(other.fd, -1);
    }
    return *this;
}

void Fd::reset(int newfd) noexcept {
#ifdef DEBUG_FD
    log("reset");
#endif
    if (fd >= 0)
        ::close(fd);
    fd = newfd;
}

int Fd::release() noexcept {
#ifdef DEBUG_FD
    log("released");
#endif
    return std::exchange(fd, -1);
}

void Fd::swap(Fd& other) noexcept {
    std::swap(fd, other.fd);
#ifdef DEBUG_FD
    log("swapped");
#endif
}

Fd Fd::dup() const {
    if (fd < 0)
        throw std::system_error(EBADF, std::generic_category(), "Fd::dup: invalid fd");

    int newfd = ::dup(fd);
    if (newfd < 0)
        throw std::system_error(errno, std::generic_category(), "Fd::dup failed");

#ifdef DEBUG_FD
    std::cerr << "[Fd] dup " << fd << " -> " << newfd << '\n';
#endif
    return Fd(newfd);
}

Fd Fd::dup2(int newfd) const {
    if (fd < 0)
        throw std::system_error(EBADF, std::generic_category(), "Fd::dup2: invalid fd");

    int res = ::dup2(fd, newfd);
    if (res < 0)
        throw std::system_error(errno, std::generic_category(), "Fd::dup2 failed");

#ifdef DEBUG_FD
    std::cerr << "[Fd] dup2 " << fd << " -> " << newfd << '\n';
#endif
    return Fd(res);
}

void Fd::redirect(int oldfd, int newfd) {
    if (::dup2(oldfd, newfd) < 0)
        throw std::system_error(errno, std::generic_category(), "Fd::redirect failed");
}

#ifdef DEBUG_FD
void Fd::log(const char* msg) const noexcept {
    std::cerr << "[Fd] fd=" << fd << " " << msg << '\n';
}
#endif