#pragma once

#include "http/HttpResponse.hpp" // for HttpResponse

#include <optional> // for optional

namespace http {

class HttpRequest;
class IHttpConnection;

class HttpClient
{
public:
    HttpClient() = default;

    std::optional<HttpResponse>
    sendRequest(const IHttpConnection& connection, const HttpRequest& request);
};

} // namespace http