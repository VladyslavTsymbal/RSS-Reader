#pragma once

#include <string>
#include <optional>

namespace http {

class HttpRequest
{
    friend class HttpRequestBuilder;

public:
    enum class HttpRequestMethod
    {
        GET,
        POST,
        PUT,
        DELETE
    };

    HttpRequestMethod
    getRequestMethod() const;

    std::string_view
    getUrl() const;

    std::string_view
    getHost() const;

private:
    HttpRequest(std::string host, std::string url, HttpRequestMethod method);

private:
    const std::string m_host;
    const std::string m_url;
    const HttpRequestMethod m_method;
};

class HttpRequestBuilder
{
public:
    HttpRequestBuilder&
    setHost(std::string host);

    HttpRequestBuilder&
    setRequestType(HttpRequest::HttpRequestMethod method);

    HttpRequestBuilder&
    setRequestUrl(std::string url);

    HttpRequest
    build();

private:
    std::string m_host;
    std::string m_url;
    HttpRequest::HttpRequestMethod m_method = HttpRequest::HttpRequestMethod::GET;
};

} // namespace http