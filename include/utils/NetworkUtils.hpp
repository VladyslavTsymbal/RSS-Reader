#pragma once

#include "utils/INetworkUtils.hpp"

namespace utils::network {

class NetworkUtils : public INetworkUtils
{
public:
    int
    createSocket(const addrinfo* addr) override;

    void
    closeSocket(const int socket) override;

    StatusCode
    connectSocket(const int socket, const addrinfo* info) override;

    std::pair<AddrInfoPtr, int>
    getAddrInfo(const std::string& ip, const std::string& port, const addrinfo* hints) override;
};

} // namespace utils::network