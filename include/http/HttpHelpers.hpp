#pragma once

#include "http/Types.hpp"
#include "http/HttpRequestMethod.hpp"

#include <optional>

namespace http {

std::size_t
findEndOfHeaders(std::string_view response_sv);

std::optional<size_t>
getContentLength(std::string_view sv);

HttpHeaders
parseHeaders(std::string_view headers_sv);

std::string
headersToString(const HttpHeaders& headers);

std::optional<std::string_view>
getValueFromHeader(const HttpHeaders& headers, std::string_view key);

} // namespace http