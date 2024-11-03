#pragma once

#include "http/HttpResponse.hpp"
#include "http/HttpRequest.hpp"
#include "utils/StatusCode.hpp"

#include <string>
#include <optional>

namespace http {

class HttpRequest;

class HttpClient
{
public:
    class HttpConnection
    {
        friend class HttpClient;

    public:
        ~HttpConnection();

        HttpConnection(HttpConnection&&) = default;
        HttpConnection&
        operator=(HttpConnection&&) = default;

        HttpConnection(const HttpConnection&) = delete;
        HttpConnection&
        operator=(const HttpConnection&) = delete;

    private:
        HttpConnection(std::string ip, const unsigned int port);

        bool
        isClosed() const;

    private:
        std::string m_ip_address;
        unsigned int m_port;
        int m_sock_fd{-1};
        bool m_is_closed{false};
    };

protected:
    utils::network::StatusCode
    sendRequest(const HttpConnection& connection, const HttpRequest& request);

    virtual utils::network::StatusCode
    sendRequestImpl(const int socket_fd, const std::string& request);

    std::string
    prepareHttpRequest(const HttpRequest& request, const std::string& ip);

    virtual std::stringstream
    getResponseImpl(const int socket_fd);

    virtual std::optional<HttpConnection>
    createConnectionImpl(std::string ip, const unsigned int port);

public:
    std::optional<HttpConnection>
    createConnection(std::string ip, const unsigned int port);

    bool
    isConnectionClosed(const HttpConnection& connection) const;

    void
    closeConnection(HttpConnection& connection);

    std::optional<HttpResponse>
    getResponse(const HttpConnection& connection, const HttpRequest& request);
};

} // namespace http