#include "http/HttpServer.hpp"
#include "utils/network/AddrInfoBuilder.hpp"
#include "utils/Log.hpp" // for LOG_ERROR
#include "utils/network/NetworkUtils.hpp"

#include <arpa/inet.h>
#include <netdb.h>
#include <string>
#include <fstream>

namespace {

const std::string_view LOG_TAG = "HttpServer";
constexpr auto MAX_REQUESTS = 6;

using utils::network::Socket;
using utils::network::INetworkUtils;

} // namespace

namespace http {

// HttpServer

HttpServer::HttpServer(
        std::string ip, const unsigned int port, std::shared_ptr<INetworkUtils> network_utils)
    : m_ip(std::move(ip))
    , m_port(port)
    , m_network_utils(std::move(network_utils))
{
    assert(m_network_utils);
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

    auto addrInfo =
            m_network_utils->getAddrInfo(m_ip, std::to_string(m_port), &hints)
                    .transform([this](AddrInfoPtr addrinfo) { m_addrinfo = std::move(addrinfo); });
    if (!addrInfo)
    {
        LOG_ERROR(LOG_TAG, "getAddrInfo failed: {}", gai_strerror(addrInfo.error()));
        return false;
    }

    m_server_socket = m_network_utils->createSocket(m_addrinfo.get());
    if (m_server_socket == nullptr)
    {
        LOG_ERROR(LOG_TAG, "createSocket failed: {}", strerror(errno));
        return false;
    }

    LOG_INFO(LOG_TAG, "Socket binding...");
    const int bind_status = ::bind(*m_server_socket, m_addrinfo->ai_addr, m_addrinfo->ai_addrlen);
    if (bind_status)
    {
        LOG_ERROR(LOG_TAG, "Socket binding failed: {}", ::strerror(errno));
        return false;
    }

    LOG_INFO(LOG_TAG, "Call `listen` for the socket.");
    const int listen_status = ::listen(*m_server_socket, MAX_REQUESTS);
    if (listen_status)
    {
        LOG_ERROR(LOG_TAG, "`Listen` failed: {}", ::strerror(errno));
        return false;
    }

    // Get rid of annoying 'Address already in use'
    int yes = 1;
    const int sockopt_status =
            ::setsockopt(*m_server_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
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
    // TODO: Enhance - sending/receiving data via connection. Remove hardcoded stuff, like sending
    // the specific file, etc.
    if (!m_initialized)
    {
        LOG_ERROR(LOG_TAG, "Server can't be started because of failed initialization.");
        return;
    }

    m_running = true;
    while (m_running)
    {
        Socket client_fd = acceptClient();
        if (client_fd == nullptr)
        {
            LOG_ERROR(LOG_TAG, "Failed to accept the client: {}", ::strerror(errno));
            continue;
        }

        LOG_INFO(LOG_TAG, "Client accepted!");

        char buffer[30000] = {0};
        ::read(*client_fd, buffer, 30000);
        LOG_DEBUG(LOG_TAG, "Data from client: {}", buffer);

        std::string request(buffer);
        if (request.find("GET /tests/feed.xml") != std::string::npos)
        {
            std::ifstream ifile("feed.xml");
            if (ifile.is_open())
            {
                std::stringstream ss;
                ss << ifile.rdbuf();

                std::string content = ss.str();
                std::string response_string =
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Type: text/html\r\n"
                        "Content-Length: " +
                        std::to_string(content.size()) +
                        "\r\n"
                        "Connection: close\r\n"
                        "\r\n" +
                        content;

                LOG_DEBUG(LOG_TAG, "Send data from server: {}", response_string);
                auto bytes = std::as_bytes(std::span(response_string));
                m_network_utils->sendBytes(client_fd, bytes);
            }
            else
            {
                LOG_ERROR(LOG_TAG, "Couldn't open \"feed.xml\"");
            }
        }
    }
}

Socket
HttpServer::acceptClient() const
{
    int client_fd = ::accept(*m_server_socket, m_addrinfo->ai_addr, &m_addrinfo->ai_addrlen);
    int* socket = new int;

    if (socket == nullptr)
    {
        return nullptr;
    }

    *socket = client_fd;
    return Socket(socket, utils::network::closeSocket);
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