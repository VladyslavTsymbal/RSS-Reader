#pragma once

#include <string_view>

namespace http {

constexpr std::string_view CRLF = "\r\n";
constexpr std::string_view CONTENT_LENGTH = "Content-Length";
constexpr std::string_view END_OF_HEADERS_SEQ = "\r\n\r\n";
constexpr std::string_view HTTP_1_1 = "HTTP/1.1";
constexpr std::string_view HOST = "Host";
constexpr std::string_view CONNECTION = "Connection";

} // namespace http
