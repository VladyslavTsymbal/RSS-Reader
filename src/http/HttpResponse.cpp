#include "http/HttpResponse.hpp"
#include "http/HttpHelpers.hpp"

#include <iterator>
#include <optional>

namespace http {

HttpResponse::HttpResponse(std::string response)
{
    if (!response.empty())
    {
        parseResponse(std::move(response));
    }
}

void
HttpResponse::parseResponse(std::string response)
{
    const auto end_of_headers_pos = findEndOfHeaders(response);
    if (end_of_headers_pos != std::string_view::npos)
    {
        // Append 4 symbols (end_of_header) \r\n\r\n at the end of the string_view
        // in order to be able successfully find the very last header.
        // Without this, string view ends just before the \r\n\r\n sequence.
        std::string_view headers_sv(response.data(), end_of_headers_pos + 4);
        // Parse status line
        m_status_line = parseStatusLine(headers_sv);

        // If status line is present, it should be skipped.
        if (m_status_line)
        {
            const auto first_crlf_pos = headers_sv.find(CRLF);
            if (first_crlf_pos != std::string_view::npos)
            {
                // Remove at the beginning N chars + (\r\n).
                headers_sv.remove_prefix(first_crlf_pos + CRLF.size());
            }
        }
        // Save headers.
        m_headers = parseHeaders(headers_sv);
    }

    // Save response body.
    auto content_length = getContentLengthValue();
    if (content_length && *content_length > 0)
    {
        // Make string_view from the `end_of_headers_pos` + `\r\n\r\n`.
        const size_t body_offset = end_of_headers_pos + 4;
        if (body_offset + *content_length <= response.size())
        {
            std::string_view body_sv(response.data() + body_offset, *content_length);
            m_body = std::string(body_sv);
        }
    }
}

std::optional<HttpResponse::StatusLine>
HttpResponse::parseStatusLine(std::string_view sv)
{
    // Expecting(example): HTTP/1.1 200 OK\r\n
    auto crlf = sv.find(CRLF);
    if (crlf != std::string_view::npos)
    {
        sv = sv.substr(0, crlf);
    }

    if (!sv.starts_with("HTTP/"))
    {
        return std::nullopt;
    }

    auto first_space = sv.find(' ');
    if (first_space == std::string_view::npos)
    {
        return std::nullopt;
    }

    auto second_space = sv.find(' ', first_space + 1);
    if (second_space == std::string_view::npos)
    {
        return std::nullopt;
    }

    StatusLine line;
    line.m_http_version = std::string(sv.substr(5, first_space - 5));

    int code{};
    auto code_sv = sv.substr(first_space + 1, second_space - first_space - 1);
    auto [ptr, ec] = std::from_chars(code_sv.data(), code_sv.data() + code_sv.size(), code);
    if (ec != std::errc{})
    {
        return std::nullopt;
    }

    line.m_status_code = code;
    line.m_description = std::string(sv.substr(second_space + 1));

    return line;
}

std::optional<std::string_view>
HttpResponse::getBody() const
{
    return m_body;
}

std::optional<std::string_view>
HttpResponse::getHeader(std::string_view key) const
{
    auto it = m_headers.find(std::string(key));
    if (it != std::end(m_headers))
    {
        return std::string_view(it->second);
    }

    return std::nullopt;
}

std::optional<size_t>
HttpResponse::getContentLengthValue() const
{
    auto value = getHeader(CONTENT_LENGTH);
    if (!value)
    {
        return std::nullopt;
    }

    // TODO: Trim string
    // auto sv = boost::trim(std::string(*value));

    size_t content_length{};
    auto [ptr, ec] = std::from_chars(value->data(), value->data() + value->size(), content_length);

    if (ec != std::errc{} || ptr != value->data() + value->size())
    {
        return std::nullopt;
    }

    return content_length;
}

std::optional<int>
HttpResponse::getStatusCode() const
{
    return m_status_line->m_status_code;
}

std::optional<std::string_view>
HttpResponse::getDescription() const
{
    return m_status_line->m_description;
}

} // namespace http