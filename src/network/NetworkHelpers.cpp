#include "network/NetworkHelpers.hpp"

namespace network {

void
copyBytes(BytesView bytes, Bytes& buffer)
{
    std::ranges::copy(std::begin(bytes), std::end(bytes), std::back_inserter(buffer));
}

BytesView
toBytesView(const std::string& s)
{
    return std::as_bytes(std::span{s});
}

std::string
bytesToString(BytesView bytes)
{
    return std::string(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}

} // namespace network
