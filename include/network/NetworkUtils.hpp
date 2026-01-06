#pragma once

#include "network/INetworkUtils.hpp"
#include "network/StatusCode.hpp"
#include "network/SysCallsWrapper.hpp"
#include "network/TcpSocket.hpp"
#include "network/Types.hpp"

namespace network {

class NetworkUtils : public INetworkUtils
{
public:
    NetworkUtils() = default;

    NetworkUtils(std::shared_ptr<ISysCallsWrapper>);

    std::optional<TcpSocket>
    createTcpSocket(const AddrInfoPtr& info) override;

    StatusCode
    connectSocket(const TcpSocket& socket, const AddrInfoPtr& info) override;

    std::expected<ConnectionData, StatusCode>
    acceptSocket(const TcpSocket& socket) const override;

    std::expected<AddrInfoPtr, int>
    getAddrInfo(std::string_view ip, Port port, const addrinfo*) override;

    std::expected<int, StatusCode>
    sendBytes(const TcpSocket& socket, BytesView bytes) const override;

    std::expected<Bytes, StatusCode>
    receiveBytes(const TcpSocket& socket, const size_t buffer_size) const override;

    std::expected<int, StatusCode>
    sendBytes(const int socket_fd, BytesView bytes) const override;

    std::expected<Bytes, StatusCode>
    receiveBytes(const int socket_fd, const size_t buffer_size) const override;

protected:
    std::shared_ptr<ISysCallsWrapper> m_syscalls_wrapper = std::make_shared<SysCallsWrapper>();
};

} // namespace network