#pragma once

#include "http/IHttpConnection.hpp"
#include "utils/network/INetworkUtils.hpp"

namespace http {

class HttpConnection : public IHttpConnection
{
public:
    HttpConnection() = delete;

    HttpConnection(
            utils::network::Socket socket,
            std::shared_ptr<utils::network::INetworkUtils> network_utils);

    HttpConnection(HttpConnection&& other) noexcept;

    HttpConnection&
    operator=(HttpConnection&& other) noexcept;

    utils::network::StatusCode
    sendBytes(std::stringstream& bytes) const override;

    std::stringstream
    receiveBytes() const override;

private:
    utils::network::Socket m_socket;
    std::shared_ptr<utils::network::INetworkUtils> m_network_utils;
};

} // namespace http