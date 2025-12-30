#include "http/HttpRequest.hpp"
#include "http/ConnectionType.hpp"
#include "http/HttpHelpers.hpp"
#include "http/Constants.hpp"

#include <format>

namespace http {

// HttpRequestBuilder

HttpRequestBuilder&
HttpRequestBuilder::setRequestType(HttpRequestMethod method)
{
    m_method = method;
    return *this;
}

HttpRequestBuilder&
HttpRequestBuilder::setRequestTarget(std::string request_target)
{
    m_request_target = std::move(request_target);
    return *this;
}

HttpRequestBuilder&
HttpRequestBuilder::setHost(std::string host)
{
    m_host = std::move(host);
    return *this;
}

HttpRequestBuilder&
HttpRequestBuilder::setConnectionType(ConnectionType connection_type)
{
    m_connection_type = connection_type;
    return *this;
}

bool
HttpRequestBuilder::isValid() const
{
    // Basic validation that REQUIRED data is present
    if (m_method == HttpRequestMethod::UNSPECIFIED)
    {
        return false;
    }

    if (m_host.empty())
    {
        return false;
    }

    if (m_request_target.empty())
    {
        return false;
    }

    return true;
}

HttpHeaders
HttpRequestBuilder::buildHttpHeaders()
{
    HttpHeaders headers;
    headers.emplace(HOST, m_host);

    if (m_connection_type)
    {
        auto connection_type_str = connectionTypeToString(*m_connection_type);
        if (connection_type_str)
        {
            headers.emplace(CONNECTION, *connection_type_str);
        }
    }

    return headers;
}

std::optional<HttpRequest>
HttpRequestBuilder::build()
{
    if (!isValid())
    {
        return std::nullopt;
    }

    if (m_headers.empty())
    {
        m_headers = buildHttpHeaders();
    }

    return HttpRequest(
            std::move(m_host),
            std::move(m_request_target),
            std::move(m_headers),
            m_method,
            std::move(m_connection_type));
}

size_t
HttpRequestBuilder::parseRequestLine(std::string_view request_line)
{
    // example: GET /index.html HTTP/1.1
    if (request_line.empty())
    {
        return std::string::npos;
    }

    // Find end of request line
    const auto control_seq_pos = request_line.find(CRLF);
    if (control_seq_pos == std::string::npos)
    {
        return std::string::npos;
    }

    // Modify string_view to look only at very first line.
    request_line = request_line.substr(0, control_seq_pos);
    // Check that request line ends with valid http version.
    if (!request_line.ends_with(HTTP_1_1))
    {
        return std::string::npos;
    }

    const auto first_whitespace = request_line.find(' ');
    // Check for first whitespace between request method and request target.
    if (first_whitespace == std::string::npos)
    {
        return std::string::npos;
    }

    std::string_view request_method_str = request_line.substr(0, first_whitespace);
    const auto request_method = stringToRequestMethod(request_method_str);
    if (request_method == HttpRequestMethod::UNSPECIFIED)
    {
        return std::string::npos;
    }

    // Save request method and modify string_view
    m_method = request_method;
    request_line = request_line.substr(first_whitespace + 1);

    const auto second_whitespace = request_line.find(' ');
    // Check for second whitespace between request target and html version.
    if (second_whitespace == std::string::npos)
    {
        return std::string::npos;
    }

    // Request target should start with '/'.
    if (!request_line.starts_with('/'))
    {
        return std::string::npos;
    }

    // Save request target
    m_request_target = std::string(std::begin(request_line), second_whitespace);
    return control_seq_pos + CRLF.size();
}

std::optional<HttpRequest>
HttpRequestBuilder::buildFromString(std::string_view request_sv)
{
    if (request_sv.empty())
    {
        return std::nullopt;
    }

    const size_t headers_pos = parseRequestLine(request_sv);
    if (headers_pos == std::string::npos)
    {
        // Start of header section is not found or parsing of the request line went wrong.
        return std::nullopt;
    }

    auto headers_sv = std::string_view(request_sv.data() + headers_pos);
    m_headers = parseHeaders(headers_sv);

    const auto host = getValueFromHeader(m_headers, HOST);
    if (!host)
    {
        return std::nullopt;
    }
    m_host = *host;

    const auto connection_type = getValueFromHeader(m_headers, CONNECTION);
    if (connection_type)
    {
        m_connection_type = stringToConnectionType(*connection_type);
    }

    return build();
}

// HttpRequest

HttpRequest::HttpRequest(
        std::string host,
        std::string uri,
        HttpHeaders headers,
        HttpRequestMethod method,
        std::optional<ConnectionType> connection_type)
    : m_host(std::move(host))
    , m_uri(std::move(uri))
    , m_headers(std::move(headers))
    , m_method{method}
    , m_connection_type(std::move(connection_type))
{
}

HttpRequestMethod
HttpRequest::getRequestMethod() const
{
    return m_method;
}

std::string_view
HttpRequest::getRequestTarget() const
{
    return m_uri;
}

std::string_view
HttpRequest::getHost() const
{
    return m_host;
}

std::optional<const ConnectionType>
HttpRequest::getConnectionType() const
{
    return m_connection_type;
}

std::optional<std::string_view>
HttpRequest::getHeader(std::string_view key) const
{
    return getValueFromHeader(m_headers, key);
}

std::string
HttpRequest::toString() const
{
    const auto request_method = requestMethodToString(m_method);
    if (!request_method)
    {
        return {};
    }

    auto headers = headersToString(m_headers);
    return std::format(
            "{0} {1} {2}{4}"
            "{3}"
            "{4}",
            *request_method,
            m_uri,
            HTTP_1_1,
            headers,
            CRLF);
}

} // namespace http