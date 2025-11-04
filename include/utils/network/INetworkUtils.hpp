#pragma once

#include "utils/network/StatusCode.hpp"
#include "utils/network/Types.hpp"

#include <expected>
#include <sstream>
#include <string_view>

namespace utils::network {

class INetworkUtils
{
public:
    virtual ~INetworkUtils() = default;

    virtual Socket
    createSocket(const addrinfo* addr) = 0;

    virtual StatusCode
    connectSocket(const Socket& socket, const addrinfo* info) = 0;

    virtual std::expected<AddrInfoPtr, int>
    getAddrInfo(std::string_view ip, std::string_view port, const addrinfo* hints) = 0;

    virtual StatusCode
    sendBytes(const Socket& socket_fd, BytesView bytes) const = 0;

    virtual std::optional<Bytes>
    receiveBytes(const Socket& socket_fd) const = 0;

protected:
    INetworkUtils() = default;
};

} // namespace utils::network