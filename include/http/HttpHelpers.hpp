#pragma once

#include "http/HttpRequest.hpp"

#include <string>
#include <optional>
#include <unordered_map>
#include <vector>

namespace http {

using HttpHeaders = std::unordered_map<std::string, std::string>;

constexpr std::string_view CRLF = "\r\n";
constexpr std::string_view CONTENT_LENGTH = "Content-Length";
constexpr std::string_view END_OF_HEADERS_SEQ = "\r\n\r\n";

std::size_t
findEndOfHeaders(std::string_view response_sv);

std::optional<size_t>
getContentLength(std::string_view sv);

std::optional<std::string>
requestMethodToString(HttpRequest::HttpRequestMethod request_method);

HttpHeaders
parseHeaders(std::string_view headers_sv);

std::string
headersToString(const HttpHeaders& headers);

} // namespace http