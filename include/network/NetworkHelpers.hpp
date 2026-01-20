#pragma once

#include "network/Types.hpp"
#include "network/TcpSocket.hpp"

#include <optional>

namespace network {

void
copyBytes(BytesView bytes, Bytes& buffer);

BytesView
toBytesView(const std::string& s);

std::string
bytesToString(BytesView bytes);

bool
makeSocketNonBlocking(const TcpSocket& socket);

std::optional<bool>
isSocketNonBlocking(const TcpSocket& socket);

} // namespace network
