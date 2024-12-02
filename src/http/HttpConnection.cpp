#include "http/HttpConnection.hpp"
#include "utils/network/INetworkUtils.hpp"
#include "utils/network/StatusCode.hpp"

namespace http {

using Socket = utils::network::Socket;
using utils::network::INetworkUtils;
using utils::network::StatusCode;

HttpConnection::HttpConnection(Socket socket, std::shared_ptr<INetworkUtils> network_utils)
    : m_socket(std::move(socket))
    , m_network_utils(std::move(network_utils))
{
}

HttpConnection::HttpConnection(HttpConnection&& other) noexcept
{
    m_socket = std::move(other.m_socket);
    m_network_utils = std::move(other.m_network_utils);
}

HttpConnection&
HttpConnection::operator=(HttpConnection&& other) noexcept
{
    m_socket = std::move(other.m_socket);
    m_network_utils = std::move(other.m_network_utils);

    return *this;
}

StatusCode
HttpConnection::sendBytes(std::stringstream& bytes) const
{
    return m_network_utils->sendBytes(*m_socket, bytes);
}

std::stringstream
HttpConnection::receiveBytes() const
{
    return m_network_utils->receiveBytes(*m_socket);
}

} // namespace http