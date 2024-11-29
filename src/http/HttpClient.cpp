#include "http/HttpClient.hpp"
#include "http/HttpConnection.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"
#include "utils/network/AddrInfoBuilder.hpp"
#include "utils/network/StatusCode.hpp"
#include "utils/Log.hpp"

#include <cstring>
#include <format>
#include <sstream>
#include <string>

namespace http {

constexpr std::string_view LOG_TAG = "HttpClient";

using utils::network::StatusCode;

HttpClient::HttpClient(std::shared_ptr<INetworkUtils> network_utils)
    : m_network_utils(std::move(network_utils))
{
}

std::unique_ptr<IHttpConnection>
HttpClient::createConnection(std::string ip, const unsigned int port)
{
    using namespace utils::network;

    const auto hints = AddrInfoBuilder()
                               .setProtocolFamily(AddrInfoBuilder::ProtocolFamily::UNSPECIFIED)
                               .setSockType(AddrInfoBuilder::SockType::TCP)
                               .build();

    const auto [result, status] = m_network_utils->getAddrInfo(ip, std::to_string(port), &hints);
    if (result == nullptr)
    {
        LOG_ERROR(LOG_TAG, "getAddrInfo failed: {}", gai_strerror(status));
        return nullptr;
    }

    const int socket_fd = m_network_utils->createSocket(result.get());
    if (socket_fd == -1)
    {
        LOG_ERROR(LOG_TAG, "createSocket failed: {}", strerror(errno));
        return nullptr;
    }

    LOG_INFO(LOG_TAG, "Connecting to: {}", ip);
    const auto is_connected = m_network_utils->connectSocket(socket_fd, result.get());
    if (is_connected == StatusCode::FAIL)
    {
        LOG_ERROR(LOG_TAG, "connectSocket failed: {}", strerror(errno));
        m_network_utils->closeSocket(socket_fd);
        return nullptr;
    }

    LOG_INFO(LOG_TAG, "Connected successfully ({})", ip);
    return std::make_unique<HttpConnection>(std::move(ip), port, socket_fd);
}

void
HttpClient::closeConnection(IHttpConnection& connection)
{
    if (!connection.isClosed())
    {
        m_network_utils->closeSocket(connection.getSocket());
        connection.closeConnection();
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
HttpClient::sendRequest(const IHttpConnection& connection, const HttpRequest& request)
{
    if (connection.isClosed())
    {
        LOG_ERROR(LOG_TAG, "Failed to send a request. Connection is already closed.");
        return StatusCode::FAIL;
    }

    const std::string request_str = prepareHttpRequest(request, connection.getUrl());
    if (request_str.empty())
    {
        LOG_ERROR(LOG_TAG, "Failed to send a request. Request method isn't valid.");
        return StatusCode::FAIL;
    }

    LOG_INFO(LOG_TAG, "Sending the request...");
    LOG_DEBUG(LOG_TAG, "Request: {}", request_str);
    const auto status = sendRequestImpl(connection.getSocket(), request_str);

    if (status == StatusCode::FAIL)
    {
        LOG_ERROR(LOG_TAG, "Send request failed: {}", strerror(errno));
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
HttpClient::getResponse(const IHttpConnection& connection, const HttpRequest& request)
{
    const auto send_status = sendRequest(connection, request);
    if (send_status == StatusCode::FAIL)
    {
        return std::nullopt;
    }

    LOG_INFO(LOG_TAG, "Receiving the response...");

    auto sstream = getResponseImpl(connection.getSocket());
    // If the stringstream has no data, that means the error is occured during receving
    // the response.
    if (sstream.rdbuf()->in_avail() == 0)
    {
        LOG_ERROR(LOG_TAG, "Getting the response failed: {}", strerror(errno));
        return std::nullopt;
    }

    return HttpResponse(std::move(sstream));
}

} // namespace http