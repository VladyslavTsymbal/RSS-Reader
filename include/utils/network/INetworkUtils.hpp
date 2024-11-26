#pragma once

#include "utils/network/StatusCode.hpp"

#include <memory>
#include <functional>
#include <netdb.h>

namespace utils::network {

using AddrInfoPtr = std::unique_ptr<addrinfo, std::function<void(addrinfo*)>>;

class INetworkUtils
{
public:
    virtual ~INetworkUtils() = default;

    virtual int
    createSocket(const addrinfo* addr) = 0;

    virtual void
    closeSocket(const int socket) = 0;

    virtual StatusCode
    connectSocket(const int socket, const addrinfo* info) = 0;

    virtual std::pair<AddrInfoPtr, int>
    getAddrInfo(const std::string& ip, const std::string& port, const addrinfo* hints) = 0;

protected:
    INetworkUtils() = default;
};

} // namespace utils::network