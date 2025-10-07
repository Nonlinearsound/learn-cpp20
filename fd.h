#ifndef FD_H
#define FD_H

#include <unistd.h>     // ::close, ::dup, ::dup2
#include <utility>      // std::exchange, std::swap
#include <system_error> // std::system_error, errno

class Fd {
    int fd;

public:
    explicit Fd(int f = -1) noexcept;
    ~Fd() noexcept;

    // no copying allowed
    Fd(const Fd&) = delete;
    Fd& operator=(const Fd&) = delete;

    // Move allowed
    Fd(Fd&& other) noexcept;
    Fd& operator=(Fd&& other) noexcept;

    // Access
    [[nodiscard]] int get() const noexcept { return fd; }
    [[nodiscard]] bool valid() const noexcept { return fd >= 0; }

    // management methods
    void reset(int newfd = -1) noexcept;
    [[nodiscard]] int release() noexcept;
    void swap(Fd& other) noexcept;

    // POSIX extension methods
    [[nodiscard]] Fd dup() const;              // <- duplicate fd
    [[nodiscard]] Fd dup2(int newfd) const;    // <- redirect fd

    // Static helper for redirect (alternative API)
    static void redirect(int oldfd, int newfd);

private:
#ifdef DEBUG_FD
    void log(const char* msg) const noexcept;
#endif
};

inline void swap(Fd& a, Fd& b) noexcept { a.swap(b); }

#endif // FD_H