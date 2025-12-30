#include "utils/network/SysCallsWrapper.hpp"

#include <netdb.h> // for addrinfo (ptr only), getaddrinfo
#include <sys/socket.h>

namespace utils::network {

int
SysCallsWrapper::socketSyscall(int domain, int type, int protocol) const
{
    return ::socket(domain, type, protocol);
}

int
SysCallsWrapper::connectSyscall(int fd, const sockaddr* addr, socklen_t len) const
{
    return ::connect(fd, addr, len);
}

int
SysCallsWrapper::acceptSyscall(int fd, sockaddr* addr, socklen_t* len) const
{
    return ::accept(fd, addr, len);
}

int
SysCallsWrapper::getaddrinfoSyscall(
        const char* __restrict name,
        const char* __restrict service,
        const addrinfo* __restrict req,
        addrinfo** __restrict pai) const
{
    return ::getaddrinfo(name, service, req, pai);
}

} // namespace utils::network