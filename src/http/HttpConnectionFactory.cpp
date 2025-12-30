#include "http/HttpConnectionFactory.hpp"
#include "http/HttpConnection.hpp"

#include "utils/network/ProtocolFamily.hpp"
#include "utils/network/Socket.hpp"
#include "utils/network/INetworkUtils.hpp"
#include "utils/network/AddrInfoBuilder.hpp"
#include "utils/network/SocketType.hpp"
#include "utils/network/StatusCode.hpp"
#include "utils/Log.hpp"

namespace {

constexpr std::string_view LOG_TAG = "HttpConnectionFactory";

using utils::network::INetworkUtils;
using utils::network::AddrInfoBuilder;
using utils::network::StatusCode;
using utils::network::TcpSocket;
using utils::network::Socket;
using utils::network::ProtocolFamily;
using utils::network::SocketType;

} // namespace

namespace http {

HttpConnectionFactory::HttpConnectionFactory(std::shared_ptr<INetworkUtils> network_utils)
    : m_network_utils(std::move(network_utils))
{
    assert(m_network_utils);
}

std::unique_ptr<IHttpConnection>
HttpConnectionFactory::createConnection(std::string_view ip, const unsigned int port)
{
    const auto hints = AddrInfoBuilder()
                               .setProtocolFamily(ProtocolFamily::UNSPECIFIED)
                               .setSockType(SocketType::TCP)
                               .build();

    auto addr_info = m_network_utils->getAddrInfo(ip, std::to_string(port), &hints);
    if (!addr_info)
    {
        LOG_ERROR(LOG_TAG, "getAddrInfo failed: {}", gai_strerror(addr_info.error()));
        return nullptr;
    }

    auto socket = m_network_utils->createTcpSocket(addr_info.value().get());
    if (!socket)
    {
        LOG_ERROR(LOG_TAG, "createSocket failed: {}", strerror(errno));
        return nullptr;
    }

    LOG_INFO(LOG_TAG, "Connecting to: {}", ip);
    const auto is_connected = m_network_utils->connectSocket(*socket, addr_info.value().get());
    if (is_connected == StatusCode::FAIL)
    {
        LOG_ERROR(LOG_TAG, "connectSocket failed: {}", strerror(errno));
        return nullptr;
    }

    LOG_INFO(LOG_TAG, "Connected successfully ({})", ip);
    return std::make_unique<HttpConnection>(std::move(*socket), m_network_utils);
}

std::unique_ptr<IHttpConnection>
HttpConnectionFactory::createConnection(TcpSocket socket)
{
    return std::make_unique<HttpConnection>(std::move(socket), m_network_utils);
}

} // namespace http