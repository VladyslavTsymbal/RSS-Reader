#include "utils/network/NetworkUtils.hpp"

namespace utils::network {

NetworkUtils::NetworkUtils(std::shared_ptr<ISysCallsWrapper> syscalls_wrapper)
    : m_syscalls_wrapper(std::move(syscalls_wrapper))
{
}

static int
closeSock(int* socket_ptr)
{
    if (socket_ptr == nullptr)
    {
        return EBADF;
    }

    return close(*socket_ptr);
}

Socket
NetworkUtils::createSocket(const addrinfo* addr)
{
    int socket_fd = -1;

    for (auto it = addr; it != nullptr; it = it->ai_next)
    {
        socket_fd = m_syscalls_wrapper->socketSyscall(
                addr->ai_family, addr->ai_socktype, addr->ai_protocol);

        if (socket_fd >= 0)
        {
            int* socket_ptr = new int;
            if (socket_ptr == nullptr)
            {
                return nullptr;
            }

            *socket_ptr = socket_fd;
            return Socket(socket_ptr, closeSock);
        }
    }

    return nullptr;
}

StatusCode
NetworkUtils::connectSocket(const int socket, const addrinfo* info)
{
    for (auto it = info; it != nullptr; it = it->ai_next)
    {
        if (m_syscalls_wrapper->connectSyscall(socket, it->ai_addr, it->ai_addrlen) == 0)
        {
            return StatusCode::OK;
        }
    }

    return StatusCode::FAIL;
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

    return AddrInfoPtr(result, freeaddrinfo);
}

StatusCode
NetworkUtils::sendBytes(const int socket_fd, std::istream& bytes) const
{
    bytes.seekg(0, std::ios::end);

    const auto request_size = bytes.tellg();
    auto bytes_to_send = request_size;

    bytes.seekg(0, std::ios::beg);

    constexpr auto BUF_SIZE = 1024;
    while (bytes_to_send != 0)
    {
        char buffer[BUF_SIZE]{0};
        auto buffer_pos = request_size - bytes_to_send;
        bytes.seekg(buffer_pos);
        bytes.read(buffer, bytes_to_send);

        auto bytes_sent = send(socket_fd, static_cast<const void*>(buffer), bytes_to_send, 0);

        if (bytes_sent == -1)
        {
            return StatusCode::FAIL;
        }

        bytes_to_send -= bytes_sent;
    }

    return StatusCode::OK;
}

std::stringstream
NetworkUtils::receiveBytes(const int socket_fd) const
{
    std::stringstream sstream;
    constexpr auto BUF_SIZE = 1024;
    char buf[BUF_SIZE];
    int bytes_received = 0;

    while ((bytes_received = recv(socket_fd, static_cast<void*>(buf), BUF_SIZE, 0)) > 0)
    {
        if (bytes_received == -1)
        {
            return {};
        }

        sstream.write(buf, bytes_received);
    }

    return sstream;
}

} // namespace utils::network