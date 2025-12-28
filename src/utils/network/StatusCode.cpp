#include "utils/network/StatusCode.hpp"

#include <unordered_map>

namespace utils::network {

std::string_view
statusCodeToError(const StatusCode code)
{
    static std::unordered_map<StatusCode, std::string_view> status_code_to_string_map = {
            {StatusCode::OK, "No errors = okay"},
            {StatusCode::FAIL, "General error, something went wrong"},
            {StatusCode::CLOSED_BY_PEER, "Connection was closed by the peer"},
            {StatusCode::READ_ERROR, "Failed to read data"},
            {StatusCode::WRITE_ERROR, "Failed to write data"},
            {StatusCode::NO_CONTENT_LENGTH, "`Content-Length` header is missing"}};

    if (status_code_to_string_map.contains(code))
    {
        return status_code_to_string_map.at(code);
    }

    return "Not known status code";
}

} // namespace utils::network