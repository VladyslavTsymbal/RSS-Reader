#pragma once

#include "network/Types.hpp"
#include "network/TcpSocket.hpp"

#include <optional>

namespace network {

void
copyBytes(Bytes& buffer, BytesView bytes);

BytesView
toBytesView(const std::string& s);

BytesView
toBytesView(std::string_view sv);

std::string_view
toStringView(BytesView bytes);

std::string_view
toStringView(const Bytes& bytes);

bool
makeSocketNonBlocking(const TcpSocket& socket);

std::optional<bool>
isSocketNonBlocking(const TcpSocket& socket);

} // namespace network
