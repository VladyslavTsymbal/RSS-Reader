#include "http/HttpResponse.hpp"
#include "http/HttpHelpers.hpp"
#include "http/Constants.hpp"
#include "network/NetworkHelpers.hpp"
#include "network/Types.hpp"

#include <optional>
#include <charconv>

namespace {

using network::Bytes;

} // namespace

namespace http {

HttpResponse::HttpResponse(Bytes bytes)
{
    if (bytes.empty())
    {
        m_is_valid = false;
        return;
    }

    m_data = std::move(bytes);
    parseResponse();
}

void
HttpResponse::parseResponse()
{
    const auto end_of_headers_pos = getEndOfHeaders(m_data);
    if (end_of_headers_pos == std::string_view::npos)
    {
        m_is_valid = false;
        return;
    }

    network::BytesView headers_bytes{m_data.data(), end_of_headers_pos};
    auto headers_sv = network::toStringView(headers_bytes);
    // Parse status line
    m_status_line = parseStatusLine(headers_sv);
    if (!m_status_line)
    {
        m_is_valid = false;
        return;
    }

    // Skip status line
    const auto first_crlf_pos = headers_sv.find(CRLF);
    if (first_crlf_pos != std::string_view::npos)
    {
        // Remove at the beginning N chars + (\r\n).
        headers_sv.remove_prefix(first_crlf_pos + CRLF.size());
    }

    // Save headers.
    m_headers = parseHeaders(headers_sv);
    if (m_headers.empty())
    {
        m_is_valid = false;
        return;
    }

    auto content_length = getContentLengthValue();
    if (content_length && *content_length > 0)
    {
        // Save body offset.
        const size_t body_offset = end_of_headers_pos + 1;
        m_body_start = body_offset;
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

    if (!sv.starts_with(HTTP_1_1))
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

std::optional<network::BytesView>
HttpResponse::getBody() const
{
    if (m_body_start)
    {
        network::BytesView body{std::begin(m_data) + *m_body_start, m_data.size() - *m_body_start};
        return body;
    }

    return std::nullopt;
}

std::optional<std::string_view>
HttpResponse::getHeader(std::string_view key) const
{
    return getValueFromHeader(m_headers, key);
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

const HttpHeaders&
HttpResponse::getHeaders() const
{
    return m_headers;
}

bool
HttpResponse::isValid() const
{
    return m_is_valid;
}

} // namespace http