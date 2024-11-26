#pragma once

#include "utils/network/ISysCallsWrapper.hpp"

namespace utils::network {

class SysCallsWrapper : public ISysCallsWrapper
{
public:
    SysCallsWrapper() = default;

    int
    socketSyscall(int domain, int type, int protocol) override;

    int
    closeSyscall(int fd) override;

    int
    connectSyscall(int fd, const sockaddr* addr, socklen_t len) override;

    int
    getaddrinfoSyscall(
            const char* __restrict name,
            const char* __restrict service,
            const addrinfo* __restrict req,
            addrinfo** __restrict pai) override;
};

} // namespace utils::network