#pragma once

#include "network/TcpSocket.hpp"
#include "network/Types.hpp"

#include <memory>
#include <string_view>

namespace network {
class INetworkUtils;
class TcpConnection;
} // namespace network

namespace http {

class HttpConnectionFactory
{
public:
    HttpConnectionFactory() = delete;
    HttpConnectionFactory(std::shared_ptr<network::INetworkUtils> network_utils);

    std::unique_ptr<network::TcpConnection>
    createTcpConnection(std::string_view ip, network::Port port);

    std::unique_ptr<network::TcpConnection>
    createTcpConnection(network::TcpSocket socket);

protected:
    std::shared_ptr<network::INetworkUtils> m_network_utils;
};

} // namespace http