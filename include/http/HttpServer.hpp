#pragma once

#include "http/HttpConnectionState.hpp"
#include "network/PollEvents.hpp"
#include "network/Types.hpp"
#include "network/TcpSocket.hpp"
#include "network/StatusCode.hpp"
#include "network/Types.hpp"

#include <string>
#include <memory>
#include <unordered_map>
#include <expected>
#include <optional>

#include <sys/poll.h>

namespace network {
class INetworkUtils;
} // namespace network

namespace http {

class HttpConnectionFactory;
class HttpConnectionHandler;
class HttpRequest;
struct PollEventDispatcher;

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
    // Utilities functions
    // Creates non-blocking socket for server
    std::optional<network::TcpSocket>
    createServerSocket(const network::AddrInfoPtr& addrinfo);

    std::expected<int, network::StatusCode>
    acceptConnection();

    void
    closeConnection(const network::Fd fd);

    void
    addPollFd(const network::Fd fd);

    void
    removePollFd(const network::Fd fd);

    void
    updatePollFdEvents(const network::Fd fd, const HandlerInterest interest);

    HttpConnectionHandler*
    findConnectionHandler(const network::Fd fd) const;

    std::optional<std::reference_wrapper<pollfd>>
    findPollFd(const network::Fd fd);

private:
    // Event-related functions
    std::vector<network::event::PollEvent>
    handlePoll();

    void
    handleAcceptEvent(const network::event::Accept& event);

    void
    handleReadEvent(const network::event::Read& event);

    void
    handleWriteEvent(const network::event::Write& event);

    void
    handleCloseEvent(const network::event::Close& event);

    void
    dispatchEvents(std::vector<network::event::PollEvent> events);

private:
    friend struct PollEventDispatcher;

    const std::string m_ip;
    const network::Port m_port;
    const std::shared_ptr<network::INetworkUtils> m_network_utils;
    const std::shared_ptr<http::HttpConnectionFactory> m_connection_factory;
    network::TcpSocket m_server_socket;
    std::vector<pollfd> m_poll_fds;
    std::unordered_map<network::Fd, std::shared_ptr<HttpConnectionHandler>> m_connection_handlers;
    bool m_initialized{false};
    bool m_running{false};
};

} // namespace http
