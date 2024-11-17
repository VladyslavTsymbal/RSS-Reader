#pragma once

#include <http/IHttpConnection.hpp>

#include <string>
#include <memory>
#include <optional>

namespace http {

class HttpResponse;
class HttpRequest;

class IHttpClient
{
public:
    virtual ~IHttpClient() = default;

    virtual std::unique_ptr<IHttpConnection>
    createConnection(std::string ip, const unsigned int port) = 0;

    virtual void
    closeConnection(IHttpConnection& connection) = 0;

    virtual std::optional<HttpResponse>
    getResponse(const IHttpConnection& connection, const HttpRequest& request) = 0;

protected:
    IHttpClient() = default;
};

} // namespace http