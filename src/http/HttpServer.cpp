#include "http/HttpServer.hpp"
#include "http/HttpConnectionFactory.hpp"
#include "http/IHttpConnection.hpp"

#include "utils/network/AddrInfoBuilder.hpp"
#include "utils/Log.hpp" // for LOG_ERROR
#include "utils/network/StatusCode.hpp"
#include "utils/network/TcpSocket.hpp"
#include "utils/network/NetworkUtils.hpp"
#include "utils/network/Types.hpp"

#include <arpa/inet.h>
#include <memory>
#include <netdb.h>
#include <string>
#include <fstream>

namespace {

constexpr std::string_view LOG_TAG = "HttpServer";
constexpr auto MAX_REQUESTS = 6;

using utils::network::Socket;
using utils::network::TcpSocket;
using utils::network::INetworkUtils;
using utils::network::StatusCode;

} // namespace

namespace http {

// HttpServer

HttpServer::HttpServer(
        std::string ip,
        const unsigned int port,
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
    using utils::network::AddrInfoBuilder;
    using utils::network::AddrInfoPtr;

    const auto hints = AddrInfoBuilder()
                               .setProtocolFamily(AddrInfoBuilder::ProtocolFamily::UNSPECIFIED)
                               .setSockType(AddrInfoBuilder::SockType::TCP)
                               .setFlags(AI_PASSIVE)
                               .build();

    auto addrinfo =
            m_network_utils->getAddrInfo(m_ip, std::to_string(m_port), &hints)
                    .transform([this](AddrInfoPtr addrinfo) { m_addrinfo = std::move(addrinfo); });
    if (!addrinfo)
    {
        LOG_ERROR(LOG_TAG, "getAddrInfo failed: {}", gai_strerror(addrinfo.error()));
        return false;
    }

    auto socket = m_network_utils->createTcpSocket(m_addrinfo.get());
    if (!socket)
    {
        LOG_ERROR(LOG_TAG, "createSocket failed: {}", strerror(errno));
        return false;
    }
    m_server_socket = std::move(*socket);

    LOG_INFO(LOG_TAG, "Socket binding...");
    const int bind_status =
            ::bind(m_server_socket.fd(), m_addrinfo->ai_addr, m_addrinfo->ai_addrlen);
    if (bind_status)
    {
        LOG_ERROR(LOG_TAG, "Socket binding failed: {}", ::strerror(errno));
        return false;
    }

    LOG_INFO(LOG_TAG, "Call `listen` for the socket.");
    const int listen_status = ::listen(m_server_socket.fd(), MAX_REQUESTS);
    if (listen_status)
    {
        LOG_ERROR(LOG_TAG, "`Listen` failed: {}", ::strerror(errno));
        return false;
    }

    // Get rid of annoying 'Address already in use'
    int yes = 1;
    const int sockopt_status =
            ::setsockopt(m_server_socket.fd(), SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    if (sockopt_status)
    {
        LOG_ERROR(LOG_TAG, "`setsockopt` failed: {}", ::strerror(errno));
        return false;
    }

    m_initialized = true;
    return true;
}

void
HttpServer::run()
{
    // Currently, this is a straight-forward implementation.
    // TODO: Remove hardcoded stuff, like sending
    // the specific file, etc.
    using utils::network::statusCodeToError;
    if (!m_initialized)
    {
        LOG_ERROR(LOG_TAG, "Server can't be started because of failed initialization.");
        return;
    }

    m_running = true;
    while (m_running)
    {
        TcpSocket client = acceptClient();
        if (!client.isValid())
        {
            LOG_ERROR(LOG_TAG, "Failed to accept the client: {}", ::strerror(errno));
            continue;
        }

        LOG_INFO(LOG_TAG, "Client accepted! Creating the connection.");
        auto connection = m_connection_factory->createConnection(std::move(client));

        auto request = connection->receiveData();
        if (!request)
        {
            const StatusCode error = request.error();
            LOG_ERROR(LOG_TAG, statusCodeToError(error));
            continue;
        }

        LOG_DEBUG(LOG_TAG, "Recieved data from client: {}", *request);

        std::string content;
        if (request->find("GET /feed.xml") != std::string::npos)
        {
            std::ifstream ifile("feed.xml");
            if (ifile.is_open())
            {
                std::stringstream ss;
                ss << ifile.rdbuf();
                content = ss.str();
            }
            else
            {
                LOG_ERROR(LOG_TAG, "Couldn't open \"feed.xml\"");
            }
        }
        else
        {
            content = R"(
                <html>
                <head><script src="https://unpkg.com/htmx.org"></script></head>
                <body>
                    <button hx-get="/feed.xml" hx-target="#result">Get Feed</button>
                    <div id="result"></div>
                </body>
                </html>
            )";
        }

        std::string response =
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: " +
                std::to_string(content.size()) +
                "\r\n"
                "Connection: close\r\n"
                "\r\n" +
                content;

        LOG_DEBUG(LOG_TAG, "Send data from server: {}", response);
        StatusCode status = connection->sendData(response);
        connection->closeConnection();

        if (status != StatusCode::OK)
        {
            LOG_ERROR(LOG_TAG, statusCodeToError(status));
            continue;
        }

        LOG_INFO(LOG_TAG, "The data was sent successfully.");
    }
}

TcpSocket
HttpServer::acceptClient() const
{
    int client_fd = ::accept(m_server_socket.fd(), m_addrinfo->ai_addr, &m_addrinfo->ai_addrlen);
    return TcpSocket(Socket(client_fd));
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

} // namespace http