#include "utils/SysCallsWrapper.hpp"
#include <unistd.h>

namespace utils::network {

int
SysCallsWrapper::socketSyscall(int domain, int type, int protocol)
{
    return socket(domain, type, protocol);
}

int
SysCallsWrapper::closeSyscall(const int socket)
{
    return close(socket);
}

int
SysCallsWrapper::connectSyscall(int fd, const sockaddr* addr, socklen_t len)
{
    return connect(fd, addr, len);
}

int
SysCallsWrapper::getaddrinfoSyscall(
        const char* __restrict name,
        const char* __restrict service,
        const addrinfo* __restrict req,
        addrinfo** __restrict pai)
{
    return getaddrinfo(name, service, req, pai);
}

} // namespace utils::network