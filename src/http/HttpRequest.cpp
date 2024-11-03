#include "http/HttpRequest.hpp"

#include <unordered_map>
#include <iostream>

namespace http {

std::optional<std::string>
requestMethodToString(HttpRequest::HttpRequestMethod request_method)
{
    static std::unordered_map<HttpRequest::HttpRequestMethod, std::string> conversion_map = {
            {HttpRequest::HttpRequestMethod::GET, "GET"},
            {HttpRequest::HttpRequestMethod::POST, "POST"},
            {HttpRequest::HttpRequestMethod::PUT, "PUT"},
            {HttpRequest::HttpRequestMethod::DELETE, "DELETE"}};

    const auto it = conversion_map.find(request_method);
    if (it == std::end(conversion_map))
    {
        std::cerr << "HttpRequest: Unexpected request method: " << static_cast<int>(request_method)
                  << '\n';
        return std::nullopt;
    }

    return it->second;
}

// HttpRequestBuilder

HttpRequestBuilder&
HttpRequestBuilder::setRequestType(HttpRequest::HttpRequestMethod method)
{
    m_method = method;
    return *this;
}

HttpRequestBuilder&
HttpRequestBuilder::setRequestUrl(std::string url)
{
    m_url = std::move(url);
    return *this;
}

HttpRequest
HttpRequestBuilder::build()
{
    return HttpRequest(m_method, std::move(m_url));
}

// HttpRequest

HttpRequest::HttpRequest(HttpRequestMethod method, std::string url)
    : m_method{method}
    , m_url(std::move(url))
{
}

HttpRequest::HttpRequestMethod
HttpRequest::getRequestMethod() const
{
    return m_method;
}

const std::string&
HttpRequest::getUrl() const
{
    return m_url;
}

} // namespace http