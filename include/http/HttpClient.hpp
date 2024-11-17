#pragma once

#include "http/IHttpClient.hpp"
#include "http/HttpResponse.hpp"
#include "utils/NetworkUtils.hpp"
#include "utils/StatusCode.hpp"

namespace http {

using utils::network::INetworkUtils;
using utils::network::NetworkUtils;

class HttpRequest;
class HttpConnection;

class HttpClient : public IHttpClient
{
public:
    HttpClient() = default;
    HttpClient(std::shared_ptr<INetworkUtils> network_utils);

    std::unique_ptr<IHttpConnection>
    createConnection(std::string ip, const unsigned int port) override;

    void
    closeConnection(IHttpConnection& connection) override;

    std::optional<HttpResponse>
    getResponse(const IHttpConnection& connection, const HttpRequest& request) override;

protected:
    utils::network::StatusCode
    sendRequest(const IHttpConnection& connection, const HttpRequest& request);

    virtual utils::network::StatusCode
    sendRequestImpl(const int socket_fd, const std::string& request);

    std::string
    prepareHttpRequest(const HttpRequest& request, const std::string& ip);

    virtual std::stringstream
    getResponseImpl(const int socket_fd);

    virtual std::unique_ptr<HttpConnection>
    createConnectionImpl(std::string ip, const unsigned int port);

protected:
    std::shared_ptr<INetworkUtils> m_network_utils = std::shared_ptr<NetworkUtils>();
};

} // namespace http