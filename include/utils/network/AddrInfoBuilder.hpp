#pragma once

#include <netdb.h>

namespace utils::network {

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
} // namespace utils::network