#pragma once

#include "http/Types.hpp"
#include "http/HttpRequestMethod.hpp"
#include "http/ConnectionType.hpp"

#include <string>
#include <optional>

namespace http {

class HttpRequest
{
    friend class HttpRequestBuilder;

public:
    HttpRequestMethod
    getRequestMethod() const;

    std::string_view
    getRequestTarget() const;

    std::string_view
    getHost() const;

    std::optional<const ConnectionType>
    getConnectionType() const;

    std::optional<std::string_view> getHeader(std::string_view) const;

    std::string
    toString() const;

private:
    HttpRequest(
            std::string,
            std::string,
            HttpHeaders,
            HttpRequestMethod,
            std::optional<ConnectionType>);

private:
    const std::string m_host;
    const std::string m_uri;
    const HttpHeaders m_headers;
    const HttpRequestMethod m_method;
    std::optional<const ConnectionType> m_connection_type;
    // TODO: Implement body
    // std::optional<std::string> m_body;
};

class HttpRequestBuilder
{
public:
    HttpRequestBuilder& setHost(std::string);

    HttpRequestBuilder& setRequestType(HttpRequestMethod);

    HttpRequestBuilder& setRequestTarget(std::string);

    HttpRequestBuilder& setConnectionType(ConnectionType);

    std::optional<HttpRequest>
    build();

    std::optional<HttpRequest> buildFromString(std::string_view);

private:
    // Returns start of the headers or npos
    size_t parseRequestLine(std::string_view);

    bool
    isValid() const;

    HttpHeaders
    buildHttpHeaders();

private:
    std::string m_host;
    std::string m_request_target;
    HttpHeaders m_headers;
    HttpRequestMethod m_method = HttpRequestMethod::UNSPECIFIED;
    std::optional<ConnectionType> m_connection_type;
};

} // namespace http