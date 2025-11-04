#pragma once

#include "utils/network/INetworkUtils.hpp"
#include <string>
#include <unistd.h>

namespace http {

class HttpServer
{
public:
    HttpServer(
            std::string ip,
            const unsigned int port,
            std::shared_ptr<utils::network::INetworkUtils> network_utils);
    ~HttpServer();

    bool
    init();

    void
    run();

    bool
    isInitialized() const;

    bool
    isRunning() const;

    utils::network::Socket
    acceptClient() const;

private:
    const std::string m_ip;
    const unsigned int m_port;
    std::shared_ptr<utils::network::INetworkUtils> m_network_utils;
    utils::network::Socket m_server_socket;
    utils::network::AddrInfoPtr m_addrinfo;
    bool m_initialized{false};
    bool m_running{false};
};

} // namespace http