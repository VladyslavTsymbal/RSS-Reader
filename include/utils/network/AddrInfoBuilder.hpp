#pragma once

#include "utils/network/SocketType.hpp"
#include "utils/network/ProtocolFamily.hpp"

#include <netdb.h> // for addrinfo

namespace utils::network {

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
} // namespace utils::network