#include "network/NetworkUtils.hpp"
#include "network/INetworkUtils.hpp"
#include "network/StatusCode.hpp"
#include "network/TcpSocket.hpp"
#include "network/Types.hpp"

#include <cstring>
#include <expected>
#include <sys/socket.h>

namespace network {

NetworkUtils::NetworkUtils(std::shared_ptr<ISysCallsWrapper> syscalls_wrapper)
    : m_syscalls_wrapper(std::move(syscalls_wrapper))
{
    assert(m_syscalls_wrapper);
}

std::optional<TcpSocket>
NetworkUtils::createTcpSocket(const AddrInfoPtr& addr)
{
    int socket_fd = -1;

    for (auto it = addr.get(); it != nullptr; it = it->ai_next)
    {
        socket_fd = m_syscalls_wrapper->socketSyscall(
                addr->ai_family, addr->ai_socktype, addr->ai_protocol);

        if (socket_fd >= 0)
        {
            return TcpSocket(Socket(socket_fd));
        }
    }

    return std::nullopt;
}

StatusCode
NetworkUtils::connectSocket(const TcpSocket& socket, const AddrInfoPtr& info)
{
    for (auto it = info.get(); it != nullptr; it = it->ai_next)
    {
        if (m_syscalls_wrapper->connectSyscall(socket.fd(), it->ai_addr, it->ai_addrlen) == 0)
        {
            return StatusCode::OK;
        }
    }

    return StatusCode::FAIL;
}

std::expected<ConnectionData, StatusCode>
NetworkUtils::acceptSocket(const TcpSocket& socket) const
{
    // AddrInfoPtr info;
    // const int socket_fd =
    //         m_syscalls_wrapper->acceptSyscall(socket.fd(), info->ai_addr, &info->ai_addrlen);
    const int socket_fd = m_syscalls_wrapper->acceptSyscall(socket.fd(), nullptr, nullptr);
    if (socket_fd >= 0)
    {
        return std::make_pair(TcpSocket(Socket(socket_fd)), std::move(nullptr));
    }
    else
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return std::unexpected(StatusCode::WOULD_BLOCK);
        }

        return std::unexpected(StatusCode::FAIL);
    }
}

std::expected<AddrInfoPtr, int>
NetworkUtils::getAddrInfo(std::string_view ip, Port port, const addrinfo* hints)
{
    int status = 0;
    addrinfo* result;

    if ((status = m_syscalls_wrapper->getaddrinfoSyscall(
                 ip.data(), std::to_string(port).c_str(), hints, &result)) != 0)
    {
        return std::unexpected(status);
    }

    return AddrInfoPtr(result, ::freeaddrinfo);
}

std::expected<int, StatusCode>
NetworkUtils::sendBytes(const int socket_fd, BytesView bytes) const
{
    const int bytes_sent = ::send(socket_fd, bytes.data(), bytes.size(), MSG_DONTWAIT);
    if (bytes_sent == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return std::unexpected(StatusCode::WOULD_BLOCK);
        }

        return std::unexpected(StatusCode::WRITE_ERROR);
    }

    return bytes_sent;
}

std::expected<int, StatusCode>
NetworkUtils::sendBytes(const TcpSocket& socket, BytesView bytes) const
{
    return sendBytes(socket.fd(), bytes);
}

std::expected<Bytes, StatusCode>
NetworkUtils::receiveBytes(const int socket_fd, const size_t buffer_size) const
{
    Bytes buffer(buffer_size);

    ssize_t received_bytes = ::recv(socket_fd, buffer.data(), buffer.size(), MSG_DONTWAIT);
    if (received_bytes == 0)
    {
        return std::unexpected(StatusCode::CLOSED_BY_PEER);
    }
    else if (received_bytes == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return std::unexpected(StatusCode::WOULD_BLOCK);
        }

        return std::unexpected(StatusCode::READ_ERROR);
    }

    buffer.resize(received_bytes);
    return buffer;
}

std::expected<Bytes, StatusCode>
NetworkUtils::receiveBytes(const TcpSocket& socket, const size_t buffer_size) const
{
    return receiveBytes(socket.fd(), buffer_size);
}

} // namespace network
