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
    createTcpSocket(const addrinfo*) = 0;

    virtual StatusCode
    connectSocket(const TcpSocket&, const addrinfo*) = 0;

    virtual std::expected<AddrInfoPtr, int>
    getAddrInfo(std::string_view ip, std::string_view port, const addrinfo* hints) = 0;

    virtual StatusCode
    sendBytes(const TcpSocket&, BytesView) const = 0;

    virtual std::expected<Bytes, StatusCode>
    receiveBytes(const TcpSocket&) const = 0;

protected:
    INetworkUtils() = default;
};

} // namespace utils::network