#include "utils/network/Socket.hpp"

namespace utils::network {

Socket::Socket(const int descriptor) noexcept
    : m_descriptor(descriptor)
{
}

Socket::~Socket()
{
    if (isValid())
    {
        ::close(m_descriptor);
    }
}

Socket::Socket(Socket&& other) noexcept
{
    if (&other != this)
    {
        std::swap(other.m_descriptor, m_descriptor);
    }
}

Socket&
Socket::operator=(Socket&& other) noexcept
{
    if (&other != this)
    {
        close();
        std::swap(other.m_descriptor, m_descriptor);
        other.m_descriptor = -1;
    }

    return *this;
}

int
Socket::fd() const noexcept
{
    return m_descriptor;
}

bool
Socket::isValid() const
{
    return m_descriptor >= 0;
}

void
Socket::close() noexcept
{
    if (isValid())
    {
        ::close(m_descriptor);
        m_descriptor = -1;
    }
}

} // namespace utils::network