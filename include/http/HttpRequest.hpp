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

    const std::string&
    getUrl() const;

private:
    HttpRequest(HttpRequestMethod method, std::string url);

private:
    const HttpRequestMethod m_method;
    const std::string m_url;
};

class HttpRequestBuilder
{
public:
    HttpRequestBuilder&
    setRequestType(HttpRequest::HttpRequestMethod method);

    HttpRequestBuilder&
    setRequestUrl(std::string url);

    HttpRequest
    build();

private:
    HttpRequest::HttpRequestMethod m_method = HttpRequest::HttpRequestMethod::GET;
    std::string m_url{'/'};
};

std::optional<std::string>
requestMethodToString(HttpRequest::HttpRequestMethod request_method);

} // namespace http