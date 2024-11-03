#include "utils/NetworkUtils.hpp"
#include "utils/StatusCode.hpp"

#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

namespace utils::network {

int
createSocket(const addrinfo* addr)
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
closeSocket(const int socket)
{
    if (socket >= 0)
    {
        close(socket);
    }
}

StatusCode
connectSocket(const int socket, const addrinfo* info)
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
getAddrInfo(const std::string& ip, const std::string& port, const addrinfo* hints)
{
    int status = 0;
    addrinfo* result;

    if ((status = getaddrinfo(ip.c_str(), port.c_str(), hints, &result)) != 0)
    {
        return std::make_pair(nullptr, status);
    }

    return std::make_pair(AddrInfoPtr(result, freeaddrinfo), status);
}

// HttpRequestBuilder

AddrInfoBuilder::AddrInfoBuilder()
{
    std::memset(&m_addrinfo, 0, sizeof(m_addrinfo));
}

AddrInfoBuilder&
AddrInfoBuilder::setProtocolFamily(AddrInfoBuilder::ProtocolFamily family)
{
    m_addrinfo.ai_family = static_cast<int>(family);
    return *this;
}

AddrInfoBuilder&
AddrInfoBuilder::setSockType(AddrInfoBuilder::SockType sock_type)
{
    m_addrinfo.ai_socktype = static_cast<int>(sock_type);
    return *this;
}

AddrInfoBuilder&
AddrInfoBuilder::setFlags(const int flags)
{
    m_addrinfo.ai_flags = flags;
    return *this;
}

addrinfo
AddrInfoBuilder::build()
{
    return m_addrinfo;
}

} // namespace utils::network