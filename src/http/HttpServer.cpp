#include "http/HttpServer.hpp"
#include "http/ConnectionType.hpp"
#include "http/Constants.hpp"
#include "http/HttpConnectionFactory.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpRequestMethod.hpp"
#include "http/HttpResponse.hpp"
#include "http/IHttpConnection.hpp"

#include "utils/network/AddrInfoBuilder.hpp"
#include "utils/Log.hpp" // for LOG_ERROR
#include "utils/network/StatusCode.hpp"
#include "utils/network/TcpSocket.hpp"
#include "utils/network/INetworkUtils.hpp"
#include "utils/network/Types.hpp"
#include "utils/network/ProtocolFamily.hpp"
#include "utils/network/SocketType.hpp"

#include <arpa/inet.h>
#include <netdb.h>
#include <fstream>

namespace {

constexpr std::string_view LOG_TAG = "HttpServer";
constexpr auto MAX_REQUESTS = 6;

using utils::network::Socket;
using utils::network::TcpSocket;
using utils::network::INetworkUtils;
using utils::network::StatusCode;
using utils::network::AddrInfoBuilder;
using utils::network::AddrInfoPtr;
using utils::network::ProtocolFamily;
using utils::network::SocketType;

constexpr std::string_view HTMX_RESPONSE =
        R"(
                <html>
                <head><script src="https://unpkg.com/htmx.org"></script></head>
                <body>
                    <button hx-get="/feed.xml" hx-target="#result">Get Feed</button>
                    <div id="result"></div>
                </body>
                </html>
            )";

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
    const auto hints = AddrInfoBuilder()
                               .setProtocolFamily(ProtocolFamily::UNSPECIFIED)
                               .setSockType(SocketType::TCP)
                               .setFlags(AI_PASSIVE)
                               .build();

    auto addrinfo =
            m_network_utils->getAddrInfo(m_ip, std::to_string(m_port), &hints)
                    .transform([this](AddrInfoPtr addrinfo) { m_addrinfo = std::move(addrinfo); });
    if (!addrinfo)
    {
        LOG_ERROR(LOG_TAG, "getAddrInfo failed: {}", ::gai_strerror(addrinfo.error()));
        return false;
    }

    auto socket = m_network_utils->createTcpSocket(m_addrinfo.get());
    if (!socket)
    {
        LOG_ERROR(LOG_TAG, "createTcpSocket failed: {}", ::strerror(errno));
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
        auto new_connection = acceptConnection();
        if (!new_connection)
        {
            LOG_ERROR(LOG_TAG, "Failed to accept the client: {}", ::strerror(errno));
            continue;
        }

        LOG_INFO(LOG_TAG, "Client accepted! Creating the connection.");
        auto request_data = new_connection->receiveData();
        if (!request_data)
        {
            const StatusCode error = request_data.error();
            LOG_ERROR(LOG_TAG, statusCodeToError(error));
            continue;
        }

        const auto request = HttpRequestBuilder().buildFromString(*request_data);
        if (!request)
        {
            LOG_ERROR(LOG_TAG, "Failed to convert received data to the valid HttpRequest");
            continue;
        }

        const auto response = createResponse(*request);
        const StatusCode status = new_connection->sendData(response);

        if (status != StatusCode::OK)
        {
            LOG_ERROR(LOG_TAG, statusCodeToError(status));
            continue;
        }

        if (shouldCloseConnection(*request))
        {
            new_connection->closeConnection();
        }

        LOG_INFO(LOG_TAG, "The data was sent successfully.");
    }
}

std::unique_ptr<IHttpConnection>
HttpServer::acceptConnection() const
{
    auto socket = m_network_utils->acceptSocket(m_server_socket, m_addrinfo);
    if (socket)
    {
        return m_connection_factory->createConnection(std::move(*socket));
    }

    return nullptr;
}

std::string
HttpServer::createResponse(const HttpRequest& request) const
{
    // TODO: Remove this hardcode and implement HttpResponseBuilder with toString method
    // This method should return HttpResponse or std::optional<HttpResponse>
    std::string response_content;
    if (request.getRequestMethod() == HttpRequestMethod::GET)
    {
        if (request.getRequestTarget() == "/feed.xml")
        {
            std::ifstream ifile("feed.xml");
            if (ifile.is_open())
            {
                std::stringstream ss;
                ss << ifile.rdbuf();
                response_content = ss.str();
            }
            else
            {
                LOG_ERROR(LOG_TAG, "Couldn't open \"feed.xml\"");
            }
        }
        else
        {
            response_content = HTMX_RESPONSE;
        }
    }

    std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: " +
            std::to_string(response_content.size()) +
            "\r\n"
            "Connection: close\r\n"
            "\r\n" +
            response_content;

    return response;
}

bool
HttpServer::shouldCloseConnection(const HttpRequest& request) const
{
    const auto connection_type = request.getConnectionType();
    if (!connection_type)
    {
        // In HTTP/1.1 connection should stay alive by default
        return false;
    }

    if (connection_type == ConnectionType::KEEP_ALIVE)
    {
        return false;
    }

    return true;
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