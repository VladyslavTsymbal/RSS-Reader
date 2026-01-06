#pragma once

#include "network/IConnection.hpp"
#include "network/INetworkUtils.hpp"
#include "network/TcpSocket.hpp"

namespace network {

class INetworkUtils;

class TcpConnection : public IConnection
{
public:
    TcpConnection() = delete;
    TcpConnection(network::TcpSocket socket, std::shared_ptr<network::INetworkUtils> network_utils);

    std::expected<int, network::StatusCode>
    sendBytes(network::BytesView bytes) const override;

    std::expected<network::Bytes, network::StatusCode>
    receiveBytes(const size_t buffer_size) const override;

    bool
    isClosed() const override;

    void
    close() override;

private:
    network::TcpSocket m_socket;
    std::shared_ptr<network::INetworkUtils> m_network_utils;
};

} // namespace network
