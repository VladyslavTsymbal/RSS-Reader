#pragma once

#include "http/Types.hpp"
#include "network/Types.hpp"

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

    HttpResponse(network::Bytes bytes);

    std::optional<network::BytesView>
    getBody() const;

    std::optional<std::string_view>
    getHeader(std::string_view key) const;

    std::optional<int>
    getStatusCode() const;

    std::optional<std::string_view>
    getDescription() const;

    const HttpHeaders&
    getHeaders() const;

    bool
    isValid() const;

private:
    void
    parseResponse();

    std::optional<StatusLine>
    parseStatusLine(std::string_view data);

    std::optional<size_t>
    getContentLengthValue() const;

private:
    network::Bytes m_data;
    HttpHeaders m_headers;
    std::optional<StatusLine> m_status_line;
    std::optional<size_t> m_body_start;
    bool m_is_valid{true};
};

} // namespace http