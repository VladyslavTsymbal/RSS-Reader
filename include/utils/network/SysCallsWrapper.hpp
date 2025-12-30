#pragma once

#include "utils/network/ISysCallsWrapper.hpp"

#include <sys/socket.h> // for socklen_t

struct addrinfo;

namespace utils::network {

class SysCallsWrapper : public ISysCallsWrapper
{
public:
    SysCallsWrapper() = default;

    int
    socketSyscall(int domain, int type, int protocol) const override;

    int
    connectSyscall(int fd, const sockaddr* addr, socklen_t len) const override;

    int
    acceptSyscall(int fd, sockaddr* addr, socklen_t* len) const override;

    // TODO: getaddrinfo is not a syscall, probably should be renamed.
    int
    getaddrinfoSyscall(
            const char* __restrict name,
            const char* __restrict service,
            const addrinfo* __restrict req,
            addrinfo** __restrict pai) const override;
};

} // namespace utils::network