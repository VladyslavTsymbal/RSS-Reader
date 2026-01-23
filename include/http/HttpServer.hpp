#pragma once

#include "network/Types.hpp"
#include "network/TcpSocket.hpp"
#include "network/StatusCode.hpp"

#include <string>
#include <memory>
#include <sys/poll.h>
#include <unordered_map>
#include <expected>
#include <optional>

namespace network {
class INetworkUtils;
} // namespace network

namespace http {

class HttpConnectionFactory;
class HttpConnectionHandler;
class HttpRequest;

class HttpServer
{
public:
    HttpServer() = delete;

    HttpServer(
            std::string ip,
            network::Port port,
            std::shared_ptr<network::INetworkUtils> network_utils,
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
    using Fd = int;
    using FdVec = std::vector<Fd>;

    // Creates non-blocking socket for server
    std::optional<network::TcpSocket>
    createServerSocket(const network::AddrInfoPtr& addrinfo);

    std::expected<int, network::StatusCode>
    acceptConnection();

    void
    closeConnection(const Fd fd);

    void
    addPollFd(const Fd fd);

    void
    removePollFd(const Fd fd);

    void
    handlePollEvents();

    void
    handleAcceptEvent(FdVec& fds_to_add);

    void
    handleReadEvent(const Fd fd, FdVec& fds_to_remove);

    void
    handleWriteEvent(const Fd fd);

    void
    handleCloseEvent(const Fd fd, FdVec& fds_to_remove);

    void
    scheduleWriteTask(const Fd fd);

    void
    removeWriteTask(const Fd fd);

    void
    processTasks();

private:
    const std::string m_ip;
    network::Port m_port;
    std::shared_ptr<network::INetworkUtils> m_network_utils;
    network::TcpSocket m_server_socket;
    std::shared_ptr<http::HttpConnectionFactory> m_connection_factory;
    std::vector<pollfd> m_poll_fds;
    std::unordered_map<Fd, std::shared_ptr<HttpConnectionHandler>> m_connections;
    FdVec m_write_queue;
    bool m_initialized{false};
    bool m_running{false};
};

} // namespace http
