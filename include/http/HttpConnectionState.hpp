#pragma once

#include <cstdint>

namespace http {

enum class HttpConnectionState : int8_t
{
    RECEIVING_HEADERS = 0,
    RECEIVING_PAYLOAD,
    WRITING,
    WAIT_TO_WRITE,
    PROCESSING,
    CLOSING,
};

} // namespace http