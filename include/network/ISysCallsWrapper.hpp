#pragma once

#include "network/Types.hpp"

#include <netdb.h>

namespace network {

class ISysCallsWrapper
{
public:
    virtual ~ISysCallsWrapper() = default;

    virtual int
    socketSyscall(int domain, int type, int protocol) const = 0;

    virtual int
    connectSyscall(Fd fd, const sockaddr* addr, socklen_t len) const = 0;

    virtual int
    acceptSyscall(Fd fd, sockaddr* addr, socklen_t* len) const = 0;

    virtual int
    getaddrinfoSyscall(
            const char* __restrict name,
            const char* __restrict service,
            const addrinfo* __restrict req,
            addrinfo** __restrict pai) const = 0;
};

} // namespace network