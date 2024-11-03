#pragma once

#include <functional>
#include <netdb.h>
#include <sys/socket.h>

#include <memory>
#include <functional>
#include <utility>

#include "utils/StatusCode.hpp"

namespace utils::network {

using AddrInfoPtr = std::unique_ptr<addrinfo, std::function<void(addrinfo*)>>;

class AddrInfoBuilder
{
public:
    enum class ProtocolFamily : int
    {
        IPv4 = AF_INET,
        IPv6 = AF_INET6,
        UNSPECIFIED = AF_UNSPEC // Both allowed
    };

    enum class SockType : int
    {
        TCP = SOCK_STREAM,
        UDP = SOCK_DGRAM
    };

public:
    AddrInfoBuilder();

    AddrInfoBuilder&
    setSockType(SockType sock_type);

    AddrInfoBuilder&
    setProtocolFamily(ProtocolFamily family);

    AddrInfoBuilder&
    setFlags(const int flags);

    addrinfo
    build();

private:
    addrinfo m_addrinfo;
};

int
createSocket(const addrinfo* addr);

void
closeSocket(const int socket);

StatusCode
connectSocket(const int socket, const addrinfo* info);

std::pair<AddrInfoPtr, int>
getAddrInfo(const std::string& ip, const std::string& port, const addrinfo* hints);

} // namespace utils::network