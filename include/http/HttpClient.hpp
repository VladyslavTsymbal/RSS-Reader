#pragma once

#include <optional> // for optional
#include <memory>

namespace http {

class HttpResponse;
class HttpRequest;
class IHttpConnection;
class HttpConnectionFactory;

class HttpClient
{
public:
    HttpClient() = default;
    HttpClient(std::shared_ptr<HttpConnectionFactory> factory);

    std::optional<HttpResponse>
    sendRequest(const HttpRequest& request);

private:
    std::shared_ptr<HttpConnectionFactory> m_factory{nullptr};
};

} // namespace http