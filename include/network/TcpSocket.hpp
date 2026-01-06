#pragma once

#include "network/Socket.hpp"

#include <cassert>
#include <memory>

namespace network {

class TcpSocket
{
public:
    TcpSocket() = default;

    TcpSocket(Socket socket)
        : m_socket(std::move(socket))
    {
        assert(!m_socket.isClosed());
    }

    TcpSocket(const TcpSocket&) = delete;
    TcpSocket&
    operator=(const TcpSocket&) = delete;

    TcpSocket(TcpSocket&&) noexcept = default;

    TcpSocket&
    operator=(TcpSocket&&) noexcept = default;

    int
    fd() const noexcept
    {
        return m_socket.fd();
    }

    void
    close() noexcept
    {
        return m_socket.close();
    }

    bool
    isValid() const noexcept
    {
        return !m_socket.isClosed();
    }

private:
    Socket m_socket;
};

} // namespace network