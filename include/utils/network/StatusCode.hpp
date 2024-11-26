#pragma once

#include <cstdint>

namespace utils::network {

enum class StatusCode : int8_t
{
    FAIL = -1,
    OK = 0
};

} // namespace utils::network