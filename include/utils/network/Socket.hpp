#pragma once

#include <algorithm>
#include <unistd.h>

namespace utils::network {

class Socket
{
public:
    Socket() = default;

    explicit Socket(const int descriptor) noexcept;

    ~Socket();

    Socket(const Socket&) = delete;

    Socket&
    operator=(const Socket&) = delete;

    Socket(Socket&& other) noexcept;

    Socket&
    operator=(Socket&& other) noexcept;

    int
    fd() const noexcept;

    bool
    isValid() const;

    void
    close() noexcept;

private:
    int m_descriptor{-1};
};

} // namespace utils::network