#include "http/HttpConnectionFactory.hpp"

#include "network/ProtocolFamily.hpp"
#include "network/INetworkUtils.hpp"
#include "network/AddrInfoBuilder.hpp"
#include "network/SocketType.hpp"
#include "network/StatusCode.hpp"
#include "network/Types.hpp"
#include "network/TcpConnection.hpp"
#include "network/TcpSocket.hpp"

#include "utils/log/Log.hpp"

namespace {

constexpr std::string_view LOG_TAG = "HttpConnectionFactory";

using network::INetworkUtils;
using network::AddrInfoBuilder;
using network::StatusCode;
using network::TcpSocket;
using network::TcpConnection;
using network::Socket;
using network::ProtocolFamily;
using network::SocketType;

} // namespace

namespace http {

HttpConnectionFactory::HttpConnectionFactory(std::shared_ptr<INetworkUtils> network_utils)
    : m_network_utils(std::move(network_utils))
{
    assert(m_network_utils);
}

std::unique_ptr<TcpConnection>
HttpConnectionFactory::createTcpConnection(std::string_view ip, network::Port port)
{
    const auto hints = AddrInfoBuilder()
                               .setProtocolFamily(ProtocolFamily::UNSPECIFIED)
                               .setSockType(SocketType::TCP)
                               .build();

    auto addr_info = m_network_utils->getAddrInfo(ip, port, &hints);
    if (!addr_info)
    {
        LOG_ERROR(LOG_TAG, "getAddrInfo failed: {}", gai_strerror(addr_info.error()));
        return nullptr;
    }

    auto socket = m_network_utils->createTcpSocket(*addr_info);
    if (!socket)
    {
        LOG_ERROR(LOG_TAG, "createSocket failed: {}", strerror(errno));
        return nullptr;
    }

    LOG_INFO(LOG_TAG, "Connecting to: {}", ip);
    const auto is_connected = m_network_utils->connectSocket(*socket, *addr_info);
    if (is_connected == StatusCode::FAIL)
    {
        LOG_ERROR(LOG_TAG, "connectSocket failed: {}", strerror(errno));
        return nullptr;
    }

    LOG_INFO(LOG_TAG, "Connected successfully ({})", ip);
    return std::make_unique<TcpConnection>(std::move(*socket), m_network_utils);
}

std::unique_ptr<TcpConnection>
HttpConnectionFactory::createTcpConnection(TcpSocket socket)
{
    return std::make_unique<TcpConnection>(std::move(socket), m_network_utils);
}

} // namespace http