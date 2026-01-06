#include "network/TcpConnection.hpp"
#include "network/StatusCode.hpp"
#include "network/INetworkUtils.hpp"
#include "network/Types.hpp"

#include <expected>

namespace {

using network::TcpSocket;
using network::INetworkUtils;
using network::BytesView;
using network::StatusCode;

} // namespace

namespace network {

TcpConnection::TcpConnection(TcpSocket socket, std::shared_ptr<INetworkUtils> network_utils)
    : m_socket(std::move(socket))
    , m_network_utils(std::move(network_utils))
{
    assert(m_socket.isValid());
    assert(m_network_utils);
}

std::expected<int, StatusCode>
TcpConnection::sendBytes(BytesView bytes) const
{
    return m_network_utils->sendBytes(m_socket, bytes);
}

std::expected<Bytes, StatusCode>
TcpConnection::receiveBytes(const size_t buffer_size) const
{
    return m_network_utils->receiveBytes(m_socket, buffer_size);
}

bool
TcpConnection::isClosed() const
{
    return !m_socket.isValid();
}

void
TcpConnection::close()
{
    m_socket.close();
}

} // namespace network
