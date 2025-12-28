#pragma once

#include "utils/network/INetworkUtils.hpp"
#include "utils/network/StatusCode.hpp"
#include "utils/network/SysCallsWrapper.hpp"
#include "utils/network/TcpSocket.hpp"
#include "utils/network/Types.hpp"

namespace utils::network {

class NetworkUtils : public INetworkUtils
{
public:
    NetworkUtils() = default;

    NetworkUtils(std::shared_ptr<ISysCallsWrapper> syscalls_wrapper);

    std::optional<TcpSocket>
    createTcpSocket(const addrinfo* addr) override;

    StatusCode
    connectSocket(const TcpSocket& socket, const addrinfo* info) override;

    std::expected<AddrInfoPtr, int>
    getAddrInfo(std::string_view ip, std::string_view port, const addrinfo* hints) override;

    StatusCode
    sendBytes(const TcpSocket& socket, BytesView bytes) const override;

    std::expected<Bytes, StatusCode>
    receiveBytes(const TcpSocket& socket) const override;

protected:
    std::shared_ptr<ISysCallsWrapper> m_syscalls_wrapper = std::make_shared<SysCallsWrapper>();
};

} // namespace utils::network