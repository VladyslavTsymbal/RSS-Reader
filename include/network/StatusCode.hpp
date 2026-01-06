#pragma once

#include <cstdint>
#include <string_view>

namespace network {

// TODO: Separate http status code from network codes
enum class StatusCode : int8_t
{
    OK = 0,
    FAIL,
    CLOSED_BY_PEER,
    READ_ERROR,
    WRITE_ERROR,
    NO_CONTENT_LENGTH,
    EXPIRED,
    WOULD_BLOCK
};

std::string_view
statusCodeToError(const StatusCode code);

} // namespace network