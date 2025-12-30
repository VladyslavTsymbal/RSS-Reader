#pragma once

#include "utils/network/Types.hpp"
#include "utils/network/TcpSocket.hpp"

#include <string>
#include <memory>

namespace utils::network {
class TcpSocket;
class INetworkUtils;
} // namespace utils::network

namespace http {

class IHttpConnection;
class HttpConnectionFactory;
class HttpRequest;

class HttpServer
{
public:
    HttpServer() = delete;

    HttpServer(
            std::string ip,
            const unsigned int port,
            std::shared_ptr<utils::network::INetworkUtils> network_utils,
            std::shared_ptr<http::HttpConnectionFactory> connection_factory);
    ~HttpServer();

    bool
    init();

    void
    run();

    bool
    isInitialized() const;

    bool
    isRunning() const;

private:
    std::unique_ptr<http::IHttpConnection>
    acceptConnection() const;

    std::string
    createResponse(const HttpRequest&) const;

    bool
    shouldCloseConnection(const HttpRequest&) const;

private:
    const std::string m_ip;
    const unsigned int m_port;
    std::shared_ptr<utils::network::INetworkUtils> m_network_utils;
    utils::network::TcpSocket m_server_socket;
    utils::network::AddrInfoPtr m_addrinfo;
    std::shared_ptr<http::HttpConnectionFactory> m_connection_factory;
    bool m_initialized{false};
    bool m_running{false};
};

} // namespace http