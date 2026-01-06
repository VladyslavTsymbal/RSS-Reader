#pragma once

#include "INonCopyable.hpp"
#include "network/IClosable.hpp"

#include <unistd.h>

namespace network {

class Socket : public IClosable, public INonCopyable
{
public:
    Socket() = default;
    ~Socket();
    explicit Socket(const int descriptor) noexcept;

    Socket(Socket&& other) noexcept;
    Socket&
    operator=(Socket&& other) noexcept;

    int
    fd() const noexcept;

    bool
    isClosed() const override;

    void
    close() override;

private:
    int m_descriptor{-1};
};

} // namespace network