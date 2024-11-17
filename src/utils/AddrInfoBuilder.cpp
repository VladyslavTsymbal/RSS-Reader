#include "utils/AddrInfoBuilder.hpp"

#include <cstring>

namespace utils::network {

AddrInfoBuilder::AddrInfoBuilder()
{
    std::memset(&m_addrinfo, 0, sizeof(m_addrinfo));
}

AddrInfoBuilder&
AddrInfoBuilder::setProtocolFamily(AddrInfoBuilder::ProtocolFamily family)
{
    m_addrinfo.ai_family = static_cast<int>(family);
    return *this;
}

AddrInfoBuilder&
AddrInfoBuilder::setSockType(AddrInfoBuilder::SockType sock_type)
{
    m_addrinfo.ai_socktype = static_cast<int>(sock_type);
    return *this;
}

AddrInfoBuilder&
AddrInfoBuilder::setFlags(const int flags)
{
    m_addrinfo.ai_flags = flags;
    return *this;
}

addrinfo
AddrInfoBuilder::build()
{
    return m_addrinfo;
}

} // namespace utils::network