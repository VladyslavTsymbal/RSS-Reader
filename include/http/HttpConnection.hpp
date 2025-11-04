#pragma once

#include "http/IHttpConnection.hpp"
#include "utils/network/INetworkUtils.hpp"
#include "utils/network/Types.hpp"

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
    sendBytes(utils::network::BytesView bytes) const override;

    std::optional<utils::network::Bytes>
    receiveBytes() const override;

private:
    utils::network::Socket m_socket;
    std::shared_ptr<utils::network::INetworkUtils> m_network_utils;
};

} // namespace http