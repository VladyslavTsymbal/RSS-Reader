#pragma once

#include "utils/network/StatusCode.hpp"
#include "utils/network/Types.hpp"
#include "utils/network/TcpSocket.hpp"

#include <expected>
#include <string_view>
#include <optional>

namespace utils::network {

class INetworkUtils
{
public:
    virtual ~INetworkUtils() = default;

    virtual std::optional<TcpSocket>
    createTcpSocket(const AddrInfoPtr& info) = 0;

    virtual StatusCode
    connectSocket(const TcpSocket& socket, const addrinfo* info) = 0;

    virtual std::optional<TcpSocket>
    acceptSocket(const TcpSocket& socket, const AddrInfoPtr& info) const = 0;

    virtual std::expected<AddrInfoPtr, int>
    getAddrInfo(std::string_view ip, Port port, const addrinfo* info) = 0;

    virtual StatusCode
    sendBytes(const TcpSocket& socket, BytesView bytes) const = 0;

    virtual std::expected<Bytes, StatusCode>
    receiveBytes(const TcpSocket& socket) const = 0;
};

} // namespace utils::network