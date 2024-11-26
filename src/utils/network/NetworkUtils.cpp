#include "utils/network/NetworkUtils.hpp"

namespace utils::network {

NetworkUtils::NetworkUtils(std::shared_ptr<ISysCallsWrapper> syscalls_wrapper)
    : m_syscalls_wrapper(std::move(syscalls_wrapper))
{
}

int
NetworkUtils::createSocket(const addrinfo* addr)
{
    int socket_fd = -1;

    for (auto it = addr; it != nullptr; it = it->ai_next)
    {
        socket_fd = m_syscalls_wrapper->socketSyscall(
                addr->ai_family, addr->ai_socktype, addr->ai_protocol);

        if (socket_fd >= 0)
        {
            return socket_fd;
        }
    }

    return socket_fd;
}

// TODO: Change return type to int
void
NetworkUtils::closeSocket(const int socket)
{
    if (socket >= 0)
    {
        m_syscalls_wrapper->closeSyscall(socket);
    }
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

std::pair<AddrInfoPtr, int>
NetworkUtils::getAddrInfo(const std::string& ip, const std::string& port, const addrinfo* hints)
{
    int status = 0;
    addrinfo* result;

    if ((status = m_syscalls_wrapper->getaddrinfoSyscall(
                 ip.c_str(), port.c_str(), hints, &result)) != 0)
    {
        return std::make_pair(nullptr, status);
    }

    return std::make_pair(AddrInfoPtr(result, freeaddrinfo), status);
}

} // namespace utils::network