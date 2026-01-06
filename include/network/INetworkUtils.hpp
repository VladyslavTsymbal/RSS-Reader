#pragma once

#include "network/StatusCode.hpp"
#include "network/Types.hpp"
#include "network/TcpSocket.hpp"

#include <expected>
#include <string_view>
#include <optional>

namespace network {

using ConnectionData = std::pair<TcpSocket, AddrInfoPtr>;

class INetworkUtils
{
public:
    virtual ~INetworkUtils() = default;

    virtual std::optional<TcpSocket>
    createTcpSocket(const AddrInfoPtr& info) = 0;

    virtual StatusCode
    connectSocket(const TcpSocket& socket, const AddrInfoPtr& info) = 0;

    virtual std::expected<ConnectionData, StatusCode>
    acceptSocket(const TcpSocket& socket) const = 0;

    virtual std::expected<AddrInfoPtr, int>
    getAddrInfo(std::string_view ip, Port port, const addrinfo* info) = 0;

    virtual std::expected<int, StatusCode>
    sendBytes(const TcpSocket& socket, BytesView bytes) const = 0;

    virtual std::expected<Bytes, StatusCode>
    receiveBytes(const TcpSocket& socket, const size_t buffer_size) const = 0;

    virtual std::expected<int, StatusCode>
    sendBytes(const int socket_fd, BytesView bytes) const = 0;

    virtual std::expected<Bytes, StatusCode>
    receiveBytes(const int socket_fd, const size_t buffer_size) const = 0;
};

} // namespace network
