#pragma once

#include <memory>
#include <string_view>

namespace utils::network {
class INetworkUtils;
} // namespace utils::network

namespace http {

class IHttpConnection;

class HttpConnectionFactory
{
public:
    HttpConnectionFactory() = delete;
    HttpConnectionFactory(std::shared_ptr<utils::network::INetworkUtils> network_utils);

    virtual std::unique_ptr<IHttpConnection>
    createConnection(std::string_view ip, const unsigned int port);

protected:
    std::shared_ptr<utils::network::INetworkUtils> m_network_utils;
};

} // namespace http