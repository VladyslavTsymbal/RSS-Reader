#pragma once

#include "utils/network/StatusCode.hpp"

#include <memory>
#include <functional>
#include <netdb.h>
#include <expected>
#include <sstream>
#include <string_view>

namespace utils::network {

using AddrInfoPtr = std::unique_ptr<addrinfo, std::function<void(addrinfo*)>>;
using Socket = std::unique_ptr<int, std::function<int(int*)>>;

class INetworkUtils
{
public:
    virtual ~INetworkUtils() = default;

    virtual Socket
    createSocket(const addrinfo* addr) = 0;

    virtual StatusCode
    connectSocket(const int socket, const addrinfo* info) = 0;

    virtual std::expected<AddrInfoPtr, int>
    getAddrInfo(std::string_view ip, std::string_view port, const addrinfo* hints) = 0;

    virtual StatusCode
    sendBytes(const int socket_fd, std::istream& bytes) const = 0;

    virtual std::stringstream
    receiveBytes(const int socket_fd) const = 0;

protected:
    INetworkUtils() = default;
};

} // namespace utils::network