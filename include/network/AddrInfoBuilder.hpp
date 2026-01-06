#pragma once

#include "network/SocketType.hpp"
#include "network/ProtocolFamily.hpp"

#include <netdb.h> // for addrinfo

namespace network {

class AddrInfoBuilder
{
public:
    AddrInfoBuilder();

    AddrInfoBuilder&
    setSockType(SocketType socket_type);

    AddrInfoBuilder&
    setProtocolFamily(ProtocolFamily protocol_family);

    AddrInfoBuilder&
    setFlags(const int flags);

    addrinfo
    build();

private:
    addrinfo m_addrinfo;
};

} // namespace network
