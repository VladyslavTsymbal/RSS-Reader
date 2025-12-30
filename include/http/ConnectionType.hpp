#pragma once

#include <cstdint>
#include <string>
#include <optional>

namespace http {

enum class ConnectionType : int8_t
{
    CLOSE = 0,
    KEEP_ALIVE,
};

std::optional<std::string>
connectionTypeToString(ConnectionType connection_type);

std::optional<ConnectionType>
stringToConnectionType(std::string_view connection_type_str);

} // namespace http