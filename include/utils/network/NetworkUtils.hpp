#pragma once

#include "utils/network/INetworkUtils.hpp"
#include "utils/network/SysCallsWrapper.hpp"

namespace utils::network {

class NetworkUtils : public INetworkUtils
{
public:
    NetworkUtils() = default;

    NetworkUtils(std::shared_ptr<ISysCallsWrapper> syscalls_wrapper);

    Socket
    createSocket(const addrinfo* addr) override;

    StatusCode
    connectSocket(const int socket, const addrinfo* info) override;

    std::expected<AddrInfoPtr, int>
    getAddrInfo(std::string_view ip, std::string_view port, const addrinfo* hints) override;

    StatusCode
    sendBytes(const int socket_fd, std::istream& bytes) const override;

    std::stringstream
    receiveBytes(const int socket_fd) const override;

protected:
    std::shared_ptr<ISysCallsWrapper> m_syscalls_wrapper = std::make_shared<SysCallsWrapper>();
};

} // namespace utils::network