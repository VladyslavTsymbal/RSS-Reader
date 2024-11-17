#include "utils/NetworkUtils.hpp"

namespace utils::network {

int
NetworkUtils::createSocket(const addrinfo* addr)
{
    int socket_fd = -1;

    for (auto it = addr; it != nullptr; it = it->ai_next)
    {
        socket_fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);

        if (socket_fd >= 0)
        {
            return socket_fd;
        }
    }

    return socket_fd;
}

void
NetworkUtils::closeSocket(const int socket)
{
    if (socket >= 0)
    {
        close(socket);
    }
}

StatusCode
NetworkUtils::connectSocket(const int socket, const addrinfo* info)
{
    for (auto it = info; it != nullptr; it = it->ai_next)
    {
        if (connect(socket, it->ai_addr, it->ai_addrlen) == 0)
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

    if ((status = getaddrinfo(ip.c_str(), port.c_str(), hints, &result)) != 0)
    {
        return std::make_pair(nullptr, status);
    }

    return std::make_pair(AddrInfoPtr(result, freeaddrinfo), status);
}

} // namespace utils::network