#pragma once

#include <cstdint>
#include <string>
#include <optional>

namespace http {

enum class HttpRequestMethod : int8_t
{
    GET = 0,
    POST,
    PUT,
    DELETE,
    UNSPECIFIED
};

std::optional<std::string>
requestMethodToString(HttpRequestMethod request_method);

HttpRequestMethod
stringToRequestMethod(std::string_view request_method_str);

} // namespace http