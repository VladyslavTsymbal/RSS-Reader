#pragma once

#include "http/Types.hpp"

#include <string>
#include <optional>

namespace http {

class HttpResponse
{
public:
    struct StatusLine
    {
        std::string m_description;
        std::string m_http_version;
        int m_status_code{};
    };

    HttpResponse(std::string response);

    std::optional<std::string_view>
    getBody() const;

    std::optional<std::string_view>
    getHeader(std::string_view key) const;

    std::optional<int>
    getStatusCode() const;

    std::optional<std::string_view>
    getDescription() const;

    const HttpHeaders&
    getHeaders() const;

private:
    void
    parseResponse(std::string response);

    std::optional<StatusLine>
    parseStatusLine(std::string_view sv);

    std::optional<size_t>
    getContentLengthValue() const;

private:
    HttpHeaders m_headers;
    std::optional<StatusLine> m_status_line;
    // TODO: Body could be binary data
    std::optional<std::string> m_body;
};

} // namespace http