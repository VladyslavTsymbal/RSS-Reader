#include "http/ConnectionType.hpp"

#include <unordered_map>

namespace http {

std::optional<std::string>
connectionTypeToString(ConnectionType connection_type)
{
    static std::unordered_map<ConnectionType, std::string> conversion_map = {
            {ConnectionType::CLOSE, "close"}, {ConnectionType::KEEP_ALIVE, "keep-alive"}};

    const auto it = conversion_map.find(connection_type);
    if (it != std::end(conversion_map))
    {
        return it->second;
    }

    return std::nullopt;
}

std::optional<ConnectionType>
stringToConnectionType(std::string_view connection_type_str)
{
    static std::unordered_map<std::string, ConnectionType> conversion_map = {
            {"close", ConnectionType::CLOSE}, {"keep-alive", ConnectionType::KEEP_ALIVE}};

    const auto it = conversion_map.find(std::string(connection_type_str));
    if (it != std::end(conversion_map))
    {
        return it->second;
    }

    return std::nullopt;
}

} // namespace http
