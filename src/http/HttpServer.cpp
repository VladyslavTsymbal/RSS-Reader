#include "http/HttpServer.hpp"
#include "http/HttpConnectionFactory.hpp"
#include "http/HttpConnectionHandler.hpp"
#include "http/HttpConnectionState.hpp"

#include "network/AddrInfoBuilder.hpp"
#include "network/NetworkHelpers.hpp"
#include "network/StatusCode.hpp"
#include "network/INetworkUtils.hpp"
#include "network/Types.hpp"
#include "network/ProtocolFamily.hpp"
#include "network/SocketType.hpp"
#include "network/TcpConnection.hpp"
#include "utils/log/Log.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <netdb.h>
#include <sys/types.h>

namespace {

constexpr std::string_view LOG_TAG = "HttpServer";
constexpr int POLL_TIMEOUT = 100;

using Port = network::Port;
using network::INetworkUtils;
using network::StatusCode;
using network::AddrInfoBuilder;
using network::AddrInfoPtr;
using network::ProtocolFamily;
using network::SocketType;
using network::TcpSocket;

} // namespace

namespace http {

// HttpServer

HttpServer::HttpServer(
        std::string ip,
        Port port,
        std::shared_ptr<INetworkUtils> network_utils,
        std::shared_ptr<HttpConnectionFactory> connection_factory)
    : m_ip(std::move(ip))
    , m_port(port)
    , m_network_utils(std::move(network_utils))
    , m_connection_factory(std::move(connection_factory))
{
    assert(m_network_utils);
    assert(m_connection_factory);
}

HttpServer::~HttpServer()
{
    m_running = false;
}

bool
HttpServer::init()
{
    if (m_initialized)
    {
        return m_initialized;
    }

    const auto hints = AddrInfoBuilder()
                               .setProtocolFamily(ProtocolFamily::UNSPECIFIED)
                               .setSockType(SocketType::TCP)
                               .setFlags(AI_PASSIVE)
                               .build();

    auto addrinfo = m_network_utils->getAddrInfo(m_ip, m_port, &hints);
    if (!addrinfo)
    {
        LOG_ERROR(LOG_TAG, "getAddrInfo failed: {}", ::gai_strerror(addrinfo.error()));
        return m_initialized;
    }

    auto socket = createServerSocket(*addrinfo);
    if (!socket)
    {
        LOG_ERROR(LOG_TAG, "Failed to create server socket");
        return m_initialized;
    }

    LOG_INFO(LOG_TAG, "Socket binding...");
    const int bind_status =
            ::bind(socket->fd(), addrinfo->get()->ai_addr, addrinfo->get()->ai_addrlen);
    if (bind_status)
    {
        LOG_ERROR(LOG_TAG, "Socket binding failed: {}", ::strerror(errno));
        return m_initialized;
    }

    LOG_INFO(LOG_TAG, "Call `listen` for the socket.");
    const int listen_status = ::listen(socket->fd(), SOMAXCONN);
    if (listen_status)
    {
        LOG_ERROR(LOG_TAG, "`Listen` failed: {}", ::strerror(errno));
        return m_initialized;
    }

    m_server_socket = std::move(*socket);
    if (!m_server_socket.isValid())
    {
        LOG_ERROR(LOG_TAG, "Failed to initialize server socket. Socket is not valid");
        return m_initialized;
    }

    addPollFd(m_server_socket.fd());
    m_initialized = true;
    return m_initialized;
}

void
HttpServer::run()
{
    if (!m_initialized)
    {
        LOG_ERROR(LOG_TAG, "Server can't be started because of failed initialization.");
        return;
    }

    m_running = true;
    while (m_running)
    {
        const int poll_events = ::poll(m_poll_fds.data(), m_poll_fds.size(), POLL_TIMEOUT);
        if (poll_events < 0)
        {
            LOG_ERROR(LOG_TAG, "Stopping. Poll ended with an error: {}", ::strerror(errno));
            m_running = false;
            break;
        }
        else if (poll_events > 0)
        {
            handlePollEvents();
        }

        processTasks();
    }
}

bool
HttpServer::isInitialized() const
{
    return m_initialized;
}

bool
HttpServer::isRunning() const
{
    return m_running;
}

std::optional<TcpSocket>
HttpServer::createServerSocket(const AddrInfoPtr& addrinfo)
{
    auto socket = m_network_utils->createTcpSocket(addrinfo);
    if (!socket || !socket->isValid())
    {
        return std::nullopt;
    }

    if (!network::makeSocketNonBlocking(*socket))
    {
        return std::nullopt;
    }

    return socket;
}

void
HttpServer::handlePollEvents()
{
    // LOG_DEBUG(LOG_TAG, "{} start", __FUNCTION__);

    const auto isReadEvent = [](const short revents) { return (revents & POLLIN); };
    const auto isWriteEvent = [](const short revents) { return (revents & POLLOUT); };
    const auto isCloseEvent = [](const short revents) { return (revents & POLLHUP); };
    const auto isAcceptEvent = [](const int server_fd, const int fd, const short revents) {
        return fd == server_fd && (revents & POLLIN);
    };

    FdVec fds_to_add;
    FdVec fds_to_remove;

    for (const auto& [fd, events, revents] : m_poll_fds)
    {
        // LOG_TRACE(LOG_TAG, "Loop :D");

        if (isAcceptEvent(m_server_socket.fd(), fd, revents))
        {
            handleAcceptEvent(fds_to_add);
        }
        else if (isReadEvent(revents))
        {
            handleReadEvent(fd, fds_to_remove);
        }
        else if (isWriteEvent(revents))
        {
            handleWriteEvent(fd);
        }
        else if (isCloseEvent(revents))
        {
            handleCloseEvent(fd, fds_to_remove);
        }
    }

    for (const Fd fd : fds_to_add)
    {
        addPollFd(fd);
    }

    for (const Fd fd : fds_to_remove)
    {
        removePollFd(fd);
    }
}

void
HttpServer::processTasks()
{
    FdVec tasks_to_remove;

    for (const Fd fd : m_pending_tasks_fd)
    {
        const auto it = m_connections.find(fd);
        if (it != std::end(m_connections) && (it->second != nullptr))
        {
            auto& handler = *it->second;
            const auto state = handler.getState();
            if (state == HttpConnectionState::PROCESSING)
            {
                handler.processData();
            }
            else if (state == HttpConnectionState::WRITING)
            {
                const auto status = handler.writeAvailable();
                if (status != StatusCode::OK)
                {
                    // Unrecoverable error
                    tasks_to_remove.push_back(fd);
                    closeConnection(fd);
                }
            }
        }
    }

    for (const Fd fd : tasks_to_remove)
    {
        removeTask(fd);
    }
}

void
HttpServer::handleAcceptEvent(FdVec& fds_to_add)
{
    while (true)
    {
        if (const auto fd = acceptConnection())
        {
            LOG_INFO(LOG_TAG, "New connection accepted! (fd: {})", *fd);
            fds_to_add.push_back(*fd);
            continue;
        }
        else if (fd.error() == StatusCode::WOULD_BLOCK)
        {
            LOG_DEBUG(LOG_TAG, "No more connection left to accept.");
            break;
        }

        LOG_ERROR(LOG_TAG, "Failed to accept an incoming connection.");
    }
}

void
HttpServer::handleReadEvent(const Fd fd, FdVec& fds_to_remove)
{
    const auto it = m_connections.find(fd);
    if ((it == std::end(m_connections)) || (it->second == nullptr))
    {
        LOG_ERROR(LOG_TAG, "Failed to read data from connection. Connection is not valid.");
        return;
    }

    auto& handler = *it->second;
    const auto status = handler.readAvailable();
    if (status != StatusCode::OK)
    {
        fds_to_remove.push_back(fd);
        closeConnection(fd);
    }
}

void
HttpServer::handleWriteEvent(const Fd fd)
{
    const auto it = m_connections.find(fd);
    if ((it == std::end(m_connections)) || (it->second == nullptr))
    {
        LOG_ERROR(LOG_TAG, "Failed to read data from connection. Connection is not valid.");
        return;
    }

    // auto& handler = *it->second;
    // FIXME: Use observer pattern to notify about that write is possible
    // state = HttpConnectionState::WRITING;

    scheduleTask(fd);
}

void
HttpServer::closeConnection(const Fd fd)
{
    if (const auto it = m_connections.find(fd); it == std::end(m_connections))
    {
        if (it->second != nullptr)
        {
            it->second->readAvailable();
            m_connections.erase(it);
            return;
        }
    }

    LOG_ERROR(LOG_TAG, "Failed to close connection gracefully. Connection is already not valid.");
}

void
HttpServer::handleCloseEvent(const Fd fd, FdVec& fds_to_remove)
{
    LOG_INFO(LOG_TAG, "Connection was closed by peer. Removing the connection.");
    fds_to_remove.push_back(fd);
    closeConnection(fd);
}

void
HttpServer::scheduleTask(const Fd fd)
{
    m_pending_tasks_fd.push_back(fd);
}

void
HttpServer::removeTask(const Fd fd)
{
    // TODO: Not optimal
    m_pending_tasks_fd.erase(
            std::remove(std::begin(m_pending_tasks_fd), std::end(m_pending_tasks_fd), fd),
            std::end(m_pending_tasks_fd));
}

std::expected<int, StatusCode>
HttpServer::acceptConnection()
{
    auto socket = m_network_utils->acceptSocket(m_server_socket);
    if (!socket)
    {
        return std::unexpected(socket.error());
    }

    auto [tcp_socket, addr_info] = std::move(*socket);
    // LOG_INFO(LOG_TAG, "Received connection from: {}", addr_info.)
    const auto socket_fd = tcp_socket.fd();
    auto tcp_connection = m_connection_factory->createTcpConnection(std::move(tcp_socket));

    if (m_connections.contains(socket_fd))
    {
        return std::unexpected(StatusCode::FAIL);
    }

    auto handler = std::make_shared<HttpConnectionHandler>(std::move(tcp_connection));
    m_connections.emplace(std::make_pair(socket_fd, std::move(handler)));
    return socket_fd;
}

void
HttpServer::addPollFd(const Fd fd)
{
    pollfd poll_fd{0, 0, 0};
    poll_fd.fd = fd;
    poll_fd.events = POLLIN | POLLOUT | POLLHUP;
    m_poll_fds.push_back(poll_fd);
}

void
HttpServer::removePollFd(const Fd fd)
{
    m_poll_fds.erase(
            std::remove_if(
                    std::begin(m_poll_fds),
                    std::end(m_poll_fds),
                    [&fd](const pollfd& poll_fd) { return poll_fd.fd == fd; }),
            std::end(m_poll_fds));
}

} // namespace http
