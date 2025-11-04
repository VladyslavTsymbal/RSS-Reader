#include "utils/network/NetworkUtils.hpp"
#include "utils/network/Types.hpp"

namespace utils::network {

NetworkUtils::NetworkUtils(std::shared_ptr<ISysCallsWrapper> syscalls_wrapper)
    : m_syscalls_wrapper(std::move(syscalls_wrapper))
{
}

int
closeSocket(int* socket_ptr)
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
            return Socket(socket_ptr, closeSocket);
        }
    }

    return nullptr;
}

StatusCode
NetworkUtils::connectSocket(const Socket& socket, const addrinfo* info)
{
    for (auto it = info; it != nullptr; it = it->ai_next)
    {
        if (m_syscalls_wrapper->connectSyscall(*socket, it->ai_addr, it->ai_addrlen) == 0)
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
NetworkUtils::sendBytes(const Socket& socket_fd, BytesView bytes) const
{
    while (!bytes.empty())
    {
        auto bytes_sent = ::send(*socket_fd, bytes.data(), bytes.size(), 0);
        if (bytes_sent == -1)
        {
            return StatusCode::FAIL;
        }

        bytes = bytes.subspan(bytes_sent);
    }

    return StatusCode::OK;
}

std::optional<Bytes>
NetworkUtils::receiveBytes(const Socket& socket_fd) const
{
    constexpr size_t buffer_size = 4096;
    std::array<std::byte, buffer_size> buffer;
    std::vector<std::byte> bytes;
    bytes.reserve(buffer_size);

    while (true)
    {
        auto received_bytes = ::recv(*socket_fd, buffer.data(), buffer.size(), 0);
        if (received_bytes == 0)
        {
            // Connection closed by peer
            break;
        }
        else if (received_bytes == -1)
        {
            // Error happened
            return std::nullopt;
        }

        bytes.insert(std::end(bytes), std::begin(buffer), std::begin(buffer) + received_bytes);
    }

    return bytes;
}

} // namespace utils::network