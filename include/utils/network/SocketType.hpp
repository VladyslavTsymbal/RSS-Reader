#pragma once

#include <sys/socket.h> // for SOCK_DGRAM, SOCK_STREAM

namespace utils::network {

enum class SocketType : int
{
    TCP = SOCK_STREAM,
    UDP = SOCK_DGRAM
};

} // namespace utils::network