#include "network/NetworkHelpers.hpp"

#include <fcntl.h>

namespace network {

void
copyBytes(Bytes& buffer, BytesView bytes)
{
    std::ranges::copy(std::begin(bytes), std::end(bytes), std::back_inserter(buffer));
}

BytesView
toBytesView(const std::string& s)
{
    return std::as_bytes(std::span{s});
}

BytesView
toBytesView(std::string_view s)
{
    return std::as_bytes(std::span{s});
}

std::string_view
toStringView(BytesView bytes)
{
    return std::string_view(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}

std::string_view
toStringView(const Bytes& bytes)
{
    return std::string_view(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}

bool
makeSocketNonBlocking(const TcpSocket& socket)
{
    const int flags = fcntl(socket.fd(), F_GETFL, 0);
    if (flags == -1)
    {
        return false;
    }

    if (fcntl(socket.fd(), F_SETFL, flags | O_NONBLOCK) == -1)
    {
        return false;
    }

    return true;
}

std::optional<bool>
isSocketNonBlocking(const TcpSocket& socket)
{
    const int flags = fcntl(socket.fd(), F_GETFL, 0);
    if (flags == -1)
    {
        return std::nullopt;
    }

    return flags & O_NONBLOCK;
}

} // namespace network
