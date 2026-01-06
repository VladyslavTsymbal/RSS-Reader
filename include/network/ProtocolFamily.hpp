#pragma once

#include <sys/socket.h> // for AF_INET, AF_INET6, AF_UNSPEC

namespace network {

enum class ProtocolFamily : int
{
    IPv4 = AF_INET,
    IPv6 = AF_INET6,
    UNSPECIFIED = AF_UNSPEC // Both allowed
};

} // namespace network
