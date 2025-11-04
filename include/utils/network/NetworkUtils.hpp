#pragma once

#include "utils/network/INetworkUtils.hpp"
#include "utils/network/SysCallsWrapper.hpp"

namespace utils::network {

int
closeSocket(int* socket);

class NetworkUtils : public INetworkUtils
{
public:
    NetworkUtils() = default;

    NetworkUtils(std::shared_ptr<ISysCallsWrapper> syscalls_wrapper);

    Socket
    createSocket(const addrinfo* addr) override;

    StatusCode
    connectSocket(const Socket& socket, const addrinfo* info) override;

    std::expected<AddrInfoPtr, int>
    getAddrInfo(std::string_view ip, std::string_view port, const addrinfo* hints) override;

    StatusCode
    sendBytes(const Socket& socket_fd, BytesView bytes) const override;

    std::optional<Bytes>
    receiveBytes(const Socket& socket_fd) const override;

protected:
    std::shared_ptr<ISysCallsWrapper> m_syscalls_wrapper = std::make_shared<SysCallsWrapper>();
};

} // namespace utils::network