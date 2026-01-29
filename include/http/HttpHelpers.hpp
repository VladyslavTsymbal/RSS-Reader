#pragma once

#include "http/Types.hpp"

#include "network/Types.hpp"

#include <optional>

namespace http {

/**
 * @brief Find the size of HTTP headers in bytes.
 *
 * This function searches for '\r\n\r\n' sequence.
 *
 * @param http_sv String view pointing to the http data.
 * @return The size of headers in bytes if '\r\n\r\n' is found
 *         std::string::npos otherwise
 */
std::size_t
getEndOfHeaders(std::string_view http_sv);

/**
 * @brief Find the size of HTTP headers in bytes.
 *
 * This function converts bytes view to the string view and
 * searches for '\r\n\r\n' sequence.
 *
 * @param buffer Bytes view to the http data.
 * @return The size of headers in bytes if '\r\n\r\n' is found
 *         std::string::npos otherwise
 */
std::size_t
getEndOfHeaders(network::BytesView buffer);

std::optional<size_t>
getContentLength(std::string_view sv);

HttpHeaders
parseHeaders(std::string_view headers_sv);

std::string
headersToString(const HttpHeaders& headers);

std::optional<std::string_view>
getValueFromHeader(const HttpHeaders& headers, std::string_view key);

} // namespace http