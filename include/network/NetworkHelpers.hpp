#pragma once

#include "network/Types.hpp"

namespace network {

void
copyBytes(BytesView bytes, Bytes& buffer);

BytesView
toBytesView(const std::string& s);

std::string
bytesToString(BytesView bytes);

} // namespace network
