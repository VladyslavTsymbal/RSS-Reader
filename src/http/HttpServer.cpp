#include "http/HttpServer.hpp"
#include "http/HttpConnectionFactory.hpp"
#include "http/HttpConnectionHandler.hpp"

#include "network/AddrInfoBuilder.hpp"
#include "network/NetworkHelpers.hpp"
#include "network/PollEvents.hpp"
#include "network/StatusCode.hpp"
#include "network/INetworkUtils.hpp"
#include "network/Types.hpp"
#include "network/ProtocolFamily.hpp"
#include "network/SocketType.hpp"
#include "network/TcpConnection.hpp"
#include "utils/log/Log.hpp"

#include <algorithm>
#include <arpa/inet.h>
#include <cerrno>
#include <netdb.h>
#include <sys/types.h>

namespace {

constexpr std::string_view LOG_TAG = "HttpServer";
constexpr int POLL_TIMEOUT = 100;

using Port = network::Port;
using network::Fd;
using network::FdVec;
using network::INetworkUtils;
using network::StatusCode;
using network::AddrInfoBuilder;
using network::AddrInfoPtr;
using network::ProtocolFamily;
using network::SocketType;
using network::TcpSocket;
using network::event::PollEvent;
using network::event::Accept;
using network::event::Read;
using network::event::Write;
using network::event::Close;
using http::HandlerInterest;

} // namespace

namespace http {

struct PollEventDispatcher
{
    HttpServer& server;

    void
    operator()(const Accept& e) const
    {
        server.handleAcceptEvent(e);
    };

    void
    operator()(const Read& e) const
    {
        server.handleReadEvent(e);
    }

    void
    operator()(const Write& e) const
    {
        server.handleWriteEvent(e);
    }

    void
    operator()(const Close& e) const
    {
        server.handleCloseEvent(e);
    }
};

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
        LOG_TRACE(LOG_TAG, "Polling...");

        const int poll_events_count = ::poll(m_poll_fds.data(), m_poll_fds.size(), POLL_TIMEOUT);
        if (poll_events_count < 0)
        {
            LOG_ERROR(LOG_TAG, "Stopping. Poll ended with an error: {}", ::strerror(errno));
            m_running = false;
            break;
        }
        else if (poll_events_count > 0)
        {
            auto poll_events = handlePoll();
            dispatchEvents(std::move(poll_events));
        }
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

    if (m_connection_handlers.contains(socket_fd))
    {
        return std::unexpected(StatusCode::FAIL);
    }

    auto handler = std::make_shared<HttpConnectionHandler>(std::move(tcp_connection));
    m_connection_handlers.emplace(socket_fd, std::move(handler));
    return socket_fd;
}

// TODO: Currently the connections are not deleted properly. When close is initiated, the connection
// should drain all data from the socket and only then the connection could be deleted. Check if the
// POLLHUP returned in such cases repeatedly and handle it. Otherwise, signal to the http server via
// callback to remove the handler entry.
void
HttpServer::closeConnection(const Fd fd)
{
    (void) fd;
    // const auto it = m_connection_handlers.find(fd);
    // if ((it == std::end(m_connection_handlers)) || (it->second == nullptr))
    // {
    //     LOG_ERROR(
    //             LOG_TAG, "Failed to close connection gracefully. Connection is already not
    //             valid.");
    //     return;
    // }

    // m_connection_handlers.erase(it);
    // removePollFd(fd);
}

void
HttpServer::addPollFd(const Fd fd)
{
    pollfd poll_fd{0, 0, 0};
    poll_fd.fd = fd;
    poll_fd.events = POLLIN | POLLHUP;
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

void
HttpServer::updatePollFdEvents(const Fd fd, const HandlerInterest interest)
{
    auto pollfd = findPollFd(fd);
    if (!pollfd)
    {
        LOG_WARN(LOG_TAG, "Failed to update poll events for fd: {}. Fd wasn't found.", fd);
        return;
    }

    // Update poll events by reference
    pollfd.value().get().events = interest.poll_interest;
}

HttpConnectionHandler*
HttpServer::findConnectionHandler(const Fd fd) const
{
    const auto it = m_connection_handlers.find(fd);
    if ((it == std::end(m_connection_handlers)) || (it->second == nullptr))
    {
        return nullptr;
    }

    return it->second.get();
}

std::optional<std::reference_wrapper<pollfd>>
HttpServer::findPollFd(const Fd fd)
{
    const auto it = std::ranges::find_if(
            m_poll_fds, [fd](const pollfd& pollfd) { return pollfd.fd == fd; });

    if (it == std::end(m_poll_fds))
    {
        return std::nullopt;
    }

    return *it;
}

std::vector<PollEvent>
HttpServer::handlePoll()
{
    const auto isReadEvent = [](const short revents) { return (revents & POLLIN); };
    const auto isWriteEvent = [](const short revents) { return (revents & POLLOUT); };
    const auto isCloseEvent = [](const short revents) { return (revents & POLLHUP); };
    const auto isAcceptEvent = [](const int server_fd, const int fd, const short revents) {
        return fd == server_fd && (revents & POLLIN);
    };

    std::vector<PollEvent> poll_events;
    for (const auto& [fd, events, revents] : m_poll_fds)
    {
        if (isAcceptEvent(m_server_socket.fd(), fd, revents))
        {
            poll_events.emplace_back(Accept{});
            continue;
        }

        if (isReadEvent(revents))
        {
            poll_events.emplace_back(Read{fd});
        }
        if (isWriteEvent(revents))
        {
            poll_events.emplace_back(Write{fd});
        }
        if (isCloseEvent(revents))
        {
            poll_events.emplace_back(Close{fd});
        }
    }

    return poll_events;
}
void
HttpServer::handleAcceptEvent(const Accept&)
{
    // Accept connection in the loop till EAGAIN.
    while (true)
    {
        if (const auto socket_fd = acceptConnection())
        {
            LOG_INFO(LOG_TAG, "New connection accepted! (fd: {})", *socket_fd);
            addPollFd(*socket_fd);
            continue;
        }
        else if (socket_fd.error() == StatusCode::WOULD_BLOCK)
        {
            LOG_TRACE(LOG_TAG, "No more connection left to accept.");
            break;
        }

        LOG_ERROR(LOG_TAG, "Failed to accept an incoming connection.");
    }
}

void
HttpServer::handleReadEvent(const Read& event)
{
    auto* connection_handler = findConnectionHandler(event.fd);
    if (!connection_handler)
    {
        LOG_ERROR(LOG_TAG, "Failed to handle read event. Connection is already not valid.");
        return;
    }

    const HandlerInterest handler_interest = connection_handler->handleEvent(event);
    updatePollFdEvents(event.fd, handler_interest);
}

void
HttpServer::handleWriteEvent(const Write& event)
{
    auto* connection_handler = findConnectionHandler(event.fd);
    if (!connection_handler)
    {
        LOG_ERROR(LOG_TAG, "Failed to handle write event. Connection is already not valid.");
        return;
    }

    const HandlerInterest handler_interest = connection_handler->handleEvent(event);
    updatePollFdEvents(event.fd, handler_interest);
}

void
HttpServer::handleCloseEvent(const Close& event)
{
    LOG_INFO(LOG_TAG, "Operating system initiated closure of the connection.");

    auto* connection_handler = findConnectionHandler(event.fd);
    if (!connection_handler)
    {
        LOG_ERROR(
                LOG_TAG, "Failed to close connection gracefully. Connection is already not valid.");
        return;
    }

    const HandlerInterest handler_interest = connection_handler->handleEvent(event);
    updatePollFdEvents(event.fd, handler_interest);
}

void
HttpServer::dispatchEvents(std::vector<PollEvent> events)
{
    for (const auto& event : events)
    {
        std::visit(PollEventDispatcher{*this}, event);
    }
}

} // namespace http
