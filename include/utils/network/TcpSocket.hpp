#pragma once

#include "utils/network/Socket.hpp"

#include <cassert>

namespace utils::network {

class TcpSocket
{
public:
    TcpSocket() = default;

    TcpSocket(Socket socket)
        : m_socket(std::move(socket))
    {
        assert(m_socket.isValid());
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
        return m_socket.isValid();
    }

private:
    Socket m_socket;
};

} // namespace utils::network