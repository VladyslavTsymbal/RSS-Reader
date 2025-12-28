#include "http/HttpRequest.hpp"

namespace http {

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

HttpRequestBuilder&
HttpRequestBuilder::setHost(std::string host)
{
    m_host = std::move(host);
    return *this;
}

HttpRequest
HttpRequestBuilder::build()
{
    return HttpRequest(std::move(m_host), std::move(m_url), m_method);
}

// HttpRequest

HttpRequest::HttpRequest(std::string host, std::string url, HttpRequestMethod method)
    : m_host(std::move(host))
    , m_url(std::move(url))
    , m_method{method}
{
}

HttpRequest::HttpRequestMethod
HttpRequest::getRequestMethod() const
{
    return m_method;
}

std::string_view
HttpRequest::getUrl() const
{
    return m_url;
}

std::string_view
HttpRequest::getHost() const
{
    return m_host;
}
} // namespace http