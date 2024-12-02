#include "http/HttpRequest.hpp"

#include <iterator>      // for end
#include <string_view>   // for string_view
#include <unordered_map> // for unordered_map, operator==, _Node_iterator_base
#include <utility>       // for move, pair
#include "utils/Log.hpp" // for LOG_ERROR

namespace http {

constexpr std::string_view LOG_TAG = "HttpRequest";

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
        LOG_ERROR(LOG_TAG, "Unexpected request method: {}", static_cast<int>(request_method));
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

const std::string&
HttpRequest::getUrl() const
{
    return m_url;
}

const std::string&
HttpRequest::getHost() const
{
    return m_host;
}
} // namespace http