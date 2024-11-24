#pragma once

#include "utils/INetworkUtils.hpp"
#include "utils/SysCallsWrapper.hpp"

namespace utils::network {

class NetworkUtils : public INetworkUtils
{
public:
    NetworkUtils() = default;

    NetworkUtils(std::shared_ptr<ISysCallsWrapper> syscalls_wrapper);

    int
    createSocket(const addrinfo* addr) override;

    void
    closeSocket(const int socket) override;

    StatusCode
    connectSocket(const int socket, const addrinfo* info) override;

    std::pair<AddrInfoPtr, int>
    getAddrInfo(const std::string& ip, const std::string& port, const addrinfo* hints) override;

protected:
    std::shared_ptr<ISysCallsWrapper> m_syscalls_wrapper = std::make_shared<SysCallsWrapper>();
};

} // namespace utils::network