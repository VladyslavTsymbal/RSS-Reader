#pragma once

#include "http/IHttpConnection.hpp"
#include "utils/network/INetworkUtils.hpp"
#include "utils/network/StatusCode.hpp"
#include "utils/network/TcpSocket.hpp"

namespace http {

class HttpConnection : public IHttpConnection
{
public:
    HttpConnection() = delete;

    HttpConnection(
            utils::network::TcpSocket socket,
            std::shared_ptr<utils::network::INetworkUtils> network_utils);

    HttpConnection(HttpConnection&& other) noexcept;

    HttpConnection&
    operator=(HttpConnection&& other) noexcept;

    utils::network::StatusCode
    sendData(std::string_view data) const override;

    std::expected<std::string, utils::network::StatusCode>
    receiveData() const override;

    bool
    isClosed() const override;

    void
    closeConnection() override;

private:
    utils::network::TcpSocket m_socket;
    std::shared_ptr<utils::network::INetworkUtils> m_network_utils;
};

} // namespace http