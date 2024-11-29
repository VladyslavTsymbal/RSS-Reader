#include "http/HttpConnection.hpp"
#include "utils/Log.hpp"

namespace http {

constexpr std::string_view LOG_TAG = "HttpConnection";

HttpConnection::HttpConnection(std::string ip, const unsigned int port, const int socket)
    : m_ip_address(std::move(ip))
    , m_port(port)
    , m_sock_fd(socket)
{
}

HttpConnection::~HttpConnection()
{
    if (!isClosed())
    {
        LOG_ERROR(
                LOG_TAG,
                "Connection wasn't closed. Please, close it with HttpClient::closeConnection.");
    }
}

HttpConnection::HttpConnection(HttpConnection&& other) noexcept
{
    m_ip_address = std::move(other.m_ip_address);
    m_port = other.m_port;
    // TODO: Check carefully this case. Socket can be still opened and we will get leak here.
    std::swap(m_sock_fd, other.m_sock_fd);
    m_is_closed = other.m_is_closed;

    other.m_port = 0;
    other.m_is_closed = true;
}

HttpConnection&
HttpConnection::operator=(HttpConnection&& other) noexcept
{
    m_ip_address = std::move(other.m_ip_address);
    m_port = other.m_port;
    // TODO: Check carefully this case. Socket can be still opened and we will get leak here.
    std::swap(m_sock_fd, other.m_sock_fd);
    m_is_closed = other.m_is_closed;

    other.m_port = 0;
    other.m_is_closed = true;

    return *this;
}

bool
HttpConnection::isClosed() const
{
    return m_is_closed;
}

void
HttpConnection::closeConnection()
{
    m_is_closed = true;
}

std::string
HttpConnection::getUrl() const
{
    return m_ip_address;
}

int
HttpConnection::getSocket() const
{
    return m_sock_fd;
}

unsigned int
HttpConnection::getPort() const
{
    return m_port;
}

} // namespace http