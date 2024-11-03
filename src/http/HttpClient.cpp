#include "http/HttpClient.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"
#include "utils/NetworkUtils.hpp"
#include "utils/StatusCode.hpp"

#include <format>
#include <sstream>
#include <iostream>
#include <string>

namespace http {

using utils::network::StatusCode;

// HttpConnection

HttpClient::HttpConnection::HttpConnection(std::string ip, const unsigned int port)
    : m_ip_address(std::move(ip))
    , m_port(port)
{
}

HttpClient::HttpConnection::~HttpConnection()
{
    if (!isClosed())
    {
        std::cerr << "HttpConnection: Connection wasn't closed. "
                     "Please, close it with HttpClient::closeConnection.\n";
    }
}

bool
HttpClient::HttpConnection::isClosed() const
{
    return m_is_closed;
}

// HttpClient

std::optional<HttpClient::HttpConnection>
HttpClient::createConnection(std::string ip, const unsigned int port)
{
    return createConnectionImpl(std::move(ip), port);
}

std::optional<HttpClient::HttpConnection>
HttpClient::createConnectionImpl(std::string ip, const unsigned int port)
{
    using namespace utils::network;

    HttpConnection connection(std::move(ip), port);

    const auto hints = AddrInfoBuilder()
                               .setProtocolFamily(AddrInfoBuilder::ProtocolFamily::UNSPECIFIED)
                               .setSockType(AddrInfoBuilder::SockType::TCP)
                               .build();

    const auto [result, status] =
            getAddrInfo(connection.m_ip_address, std::to_string(connection.m_port), &hints);
    if (result == nullptr)
    {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << '\n';
        return std::nullopt;
    }

    connection.m_sock_fd = createSocket(result.get());
    if (connection.m_sock_fd == -1)
    {
        perror("socket");
        return std::nullopt;
    }

    std::cout << "HttpConnection: Connecting to: " << connection.m_ip_address << '\n';
    const auto is_connected = connectSocket(connection.m_sock_fd, result.get());
    if (is_connected == StatusCode::FAIL)
    {
        perror("connect");
        closeSocket(connection.m_sock_fd);
        return std::nullopt;
    }

    std::cout << "HttpConnection: Connected successfully (" << connection.m_ip_address << ")\n";
    // TODO: Is it copied here???
    return connection;
}

bool
HttpClient::isConnectionClosed(const HttpConnection& connection) const
{
    return connection.isClosed();
}

void
HttpClient::closeConnection(HttpConnection& connection)
{
    if (!connection.isClosed())
    {
        utils::network::closeSocket(connection.m_sock_fd);
        connection.m_is_closed = true;
    }
}

std::string
HttpClient::prepareHttpRequest(const HttpRequest& request, const std::string& ip)
{
    const auto request_method = requestMethodToString(request.getRequestMethod());
    if (!request_method)
    {
        return {};
    }

    return std::format(
            "{} {} HTTP/1.1\x0D\x0AHost: {}\x0D\x0A\x43onnection: Close\x0D\x0A\x0D\x0A",
            *request_method,
            request.getUrl(),
            ip);
}

StatusCode
HttpClient::sendRequestImpl(const int socket_fd, const std::string& request)
{
    const auto request_size = request.size();
    auto bytes_to_send = request_size;

    while (bytes_to_send != 0)
    {
        decltype(bytes_to_send) buffer_pos = request_size - bytes_to_send;
        auto bytes_sent =
                send(socket_fd, static_cast<const void*>(&request[buffer_pos]), request_size, 0);

        if (bytes_sent == -1)
        {
            return StatusCode::FAIL;
        }

        bytes_to_send -= bytes_sent;
    }

    return StatusCode::OK;
}

StatusCode
HttpClient::sendRequest(const HttpConnection& connection, const HttpRequest& request)
{
    if (connection.isClosed())
    {
        std::cerr << "HttpClient: Unable to send request. Connection is already closed.\n";
        return StatusCode::FAIL;
    }

    const std::string request_str = prepareHttpRequest(request, connection.m_ip_address);
    if (request_str.empty())
    {
        std::cerr << "HttpClient: Unable to send a request. Request method isn't valid.\n";
        return StatusCode::FAIL;
    }

    std::cout << "HttpClient: Sending the request:\n" << request_str << '\n';
    const auto status = sendRequestImpl(connection.m_sock_fd, request_str);

    if (status == StatusCode::FAIL)
    {
        perror("send error");
    }

    return status;
}

std::stringstream
HttpClient::getResponseImpl(const int socket_fd)
{
    std::stringstream sstream;
    constexpr auto BUF_SIZE = 1024;
    char buf[BUF_SIZE];
    int bytes_received = 0;

    while ((bytes_received = recv(socket_fd, static_cast<void*>(buf), BUF_SIZE, 0)) > 0)
    {
        if (bytes_received == -1)
        {
            return {};
        }

        sstream.write(buf, bytes_received);
    }

    return sstream;
}

std::optional<HttpResponse>
HttpClient::getResponse(const HttpConnection& connection, const HttpRequest& request)
{
    const auto send_status = sendRequest(connection, request);
    if (send_status == StatusCode::FAIL)
    {
        return std::nullopt;
    }

    std::cout << "HttpClient: Receiving the response\n";

    auto sstream = getResponseImpl(connection.m_sock_fd);
    // If the stringstream has no data, that means the error is occured during receving
    // the response.
    if (sstream.rdbuf()->in_avail() == 0)
    {
        perror("recv");
        return std::nullopt;
    }

    return HttpResponse(std::move(sstream));
}

} // namespace http