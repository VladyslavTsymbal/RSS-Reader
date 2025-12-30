#include "http/HttpHelpers.hpp"
#include "http/Constants.hpp"

#include <charconv>

namespace http {

std::size_t
findEndOfHeaders(std::string_view response_sv)
{
    return response_sv.find(END_OF_HEADERS_SEQ);
}

std::optional<size_t>
getContentLength(std::string_view sv)
{
    auto pos = sv.find(CONTENT_LENGTH);
    if (pos == std::string_view::npos)
    {
        return std::nullopt;
    }

    pos += CONTENT_LENGTH.size();

    // Checking for ':' symbol after header
    if (pos >= sv.size() || sv[pos] != ':')
    {
        return std::nullopt;
    }

    ++pos;

    // Skip whitespaces
    while (pos < sv.size() && sv[pos] == ' ')
    {
        ++pos;
    }

    auto end = sv.find(CRLF, pos);
    if (end == std::string_view::npos)
    {
        return std::nullopt;
    }

    size_t value{};
    auto [ptr, ec] = std::from_chars(sv.data() + pos, sv.data() + end, value);

    if (ec != std::errc{} || ptr != sv.data() + end)
    {
        return std::nullopt;
    }

    return value;
}

HttpHeaders
parseHeaders(std::string_view headers_sv)
{
    HttpHeaders http_headers;

    while (!headers_sv.empty())
    {
        const auto colon_pos = headers_sv.find(':');
        if (colon_pos == std::string_view::npos)
        {
            break;
        }

        const auto value_start_pos = headers_sv.find_first_not_of(' ', colon_pos + 1);
        if (value_start_pos == std::string_view::npos)
        {
            break;
        }

        const auto value_end_pos = headers_sv.find(CRLF, value_start_pos);
        if (value_end_pos == std::string_view::npos)
        {
            break;
        }

        std::string key(std::begin(headers_sv), colon_pos);
        std::string value(
                std::begin(headers_sv) + value_start_pos, value_end_pos - value_start_pos);
        if (!http_headers.contains(key))
        {
            http_headers.emplace(std::make_pair(std::move(key), std::move(value)));
        }

        headers_sv.remove_prefix(value_end_pos + CRLF.size());
    }

    return http_headers;
}

std::string
headersToString(const HttpHeaders& headers)
{
    if (headers.empty())
    {
        return {};
    }

    std::string str;
    str.reserve(512);

    for (const auto& [key, value] : headers)
    {
        str.append(key);
        str.append(": ");
        str.append(value);
        str.append(CRLF);
    }

    return str;
}

std::optional<std::string_view>
getValueFromHeader(const HttpHeaders& headers, std::string_view key)
{
    auto it = headers.find(std::string(key));
    if (it != std::end(headers))
    {
        return std::string_view(it->second);
    }

    return std::nullopt;
}

} // namespace http