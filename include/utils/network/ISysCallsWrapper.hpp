#pragma once

#include <netdb.h>

namespace utils::network {

class ISysCallsWrapper
{
public:
    virtual ~ISysCallsWrapper() = default;

    virtual int
    socketSyscall(int domain, int type, int protocol) = 0;

    virtual int
    connectSyscall(int fd, const sockaddr* addr, socklen_t len) = 0;

    virtual int
    getaddrinfoSyscall(
            const char* __restrict name,
            const char* __restrict service,
            const addrinfo* __restrict req,
            addrinfo** __restrict pai) = 0;

protected:
    ISysCallsWrapper() = default;
};

} // namespace utils::network