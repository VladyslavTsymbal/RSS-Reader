#include "utils/network/NetworkUtils.hpp"
#include "utils/network/StatusCode.hpp"
#include "utils/network/TcpSocket.hpp"
#include <cstring>

namespace utils::network {

NetworkUtils::NetworkUtils(std::shared_ptr<ISysCallsWrapper> syscalls_wrapper)
    : m_syscalls_wrapper(std::move(syscalls_wrapper))
{
    assert(m_syscalls_wrapper);
}

std::optional<TcpSocket>
NetworkUtils::createTcpSocket(const addrinfo* addr)
{
    int socket_fd = -1;

    for (auto it = addr; it != nullptr; it = it->ai_next)
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
NetworkUtils::connectSocket(const TcpSocket& socket, const addrinfo* info)
{
    for (auto it = info; it != nullptr; it = it->ai_next)
    {
        if (m_syscalls_wrapper->connectSyscall(socket.fd(), it->ai_addr, it->ai_addrlen) == 0)
        {
            return StatusCode::OK;
        }
    }

    return StatusCode::FAIL;
}

std::optional<TcpSocket>
NetworkUtils::acceptSocket(const TcpSocket& socket, const AddrInfoPtr& addrinfo) const
{
    const int socket_fd = m_syscalls_wrapper->acceptSyscall(
            socket.fd(), addrinfo->ai_addr, &addrinfo->ai_addrlen);
    if (socket_fd >= 0)
    {
        return TcpSocket(Socket(socket_fd));
    }

    return std::nullopt;
}

std::expected<AddrInfoPtr, int>
NetworkUtils::getAddrInfo(std::string_view ip, std::string_view port, const addrinfo* hints)
{
    int status = 0;
    addrinfo* result;

    if ((status = m_syscalls_wrapper->getaddrinfoSyscall(ip.data(), port.data(), hints, &result)) !=
        0)
    {
        return std::unexpected(status);
    }

    return AddrInfoPtr(result, ::freeaddrinfo);
}

StatusCode
NetworkUtils::sendBytes(const TcpSocket& socket, BytesView bytes) const
{
    while (!bytes.empty())
    {
        auto bytes_sent = ::send(socket.fd(), bytes.data(), bytes.size(), 0);
        if (bytes_sent == -1)
        {
            return StatusCode::WRITE_ERROR;
        }

        bytes = bytes.subspan(bytes_sent);
    }

    return StatusCode::OK;
}

std::expected<Bytes, StatusCode>
NetworkUtils::receiveBytes(const TcpSocket& socket) const
{
    constexpr size_t buffer_size = 4096;
    std::array<std::byte, buffer_size> buffer;

    ssize_t received_bytes = 0;
    received_bytes = ::recv(socket.fd(), buffer.data(), buffer.size(), 0);
    if (received_bytes == 0)
    {
        return std::unexpected(StatusCode::CLOSED_BY_PEER);
    }
    else if (received_bytes == -1)
    {
        return std::unexpected(StatusCode::READ_ERROR);
    }

    std::vector<std::byte> bytes;
    bytes.resize(received_bytes);
    std::memcpy(bytes.data(), buffer.data(), received_bytes);

    return bytes;
}

} // namespace utils::network