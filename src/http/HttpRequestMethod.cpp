#include "http/HttpRequestMethod.hpp"

#include <unordered_map>

namespace http {

std::optional<std::string>
requestMethodToString(HttpRequestMethod request_method)
{
    static std::unordered_map<HttpRequestMethod, std::string> conversion_map = {
            {HttpRequestMethod::GET, "GET"},
            {HttpRequestMethod::POST, "POST"},
            {HttpRequestMethod::PUT, "PUT"},
            {HttpRequestMethod::DELETE, "DELETE"},
            {HttpRequestMethod::UNSPECIFIED, "UNSPECIFIED"}};

    const auto it = conversion_map.find(request_method);
    if (it != std::end(conversion_map))
    {
        return it->second;
    }

    return std::nullopt;
}

HttpRequestMethod
stringToRequestMethod(std::string_view request_method_str)
{
    static std::unordered_map<std::string, HttpRequestMethod> conversion_map = {
            {"GET", HttpRequestMethod::GET},
            {"POST", HttpRequestMethod::POST},
            {"PUT", HttpRequestMethod::PUT},
            {"DELETE", HttpRequestMethod::DELETE},
            {"UNSPECIFIED", HttpRequestMethod::UNSPECIFIED}};

    const auto it = conversion_map.find(std::string(request_method_str));
    if (it != std::end(conversion_map))
    {
        return it->second;
    }

    return HttpRequestMethod::UNSPECIFIED;
}

} // namespace http