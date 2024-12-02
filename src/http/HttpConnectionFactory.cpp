#include "http/HttpConnectionFactory.hpp"
#include "http/HttpConnection.hpp"
#include "utils/network/INetworkUtils.hpp"
#include "utils/network/AddrInfoBuilder.hpp"
#include "utils/Log.hpp"

namespace http {

constexpr std::string_view LOG_TAG = "HttpConnectionFactory";

using namespace utils::network;

HttpConnectionFactory::HttpConnectionFactory(std::shared_ptr<INetworkUtils> network_utils)
    : m_network_utils(std::move(network_utils))
{
}

std::unique_ptr<IHttpConnection>
HttpConnectionFactory::createConnection(std::string_view ip, const unsigned int port)
{
    const auto hints = AddrInfoBuilder()
                               .setProtocolFamily(AddrInfoBuilder::ProtocolFamily::UNSPECIFIED)
                               .setSockType(AddrInfoBuilder::SockType::TCP)
                               .build();

    const auto ptrOrErrorCode = m_network_utils->getAddrInfo(ip, std::to_string(port), &hints);
    if (!ptrOrErrorCode.has_value())
    {
        LOG_ERROR(LOG_TAG, "getAddrInfo failed: {}", gai_strerror(ptrOrErrorCode.error()));
        return nullptr;
    }

    Socket socket = m_network_utils->createSocket(ptrOrErrorCode.value().get());
    if (socket == nullptr)
    {
        LOG_ERROR(LOG_TAG, "createSocket failed: {}", strerror(errno));
        return nullptr;
    }

    LOG_INFO(LOG_TAG, "Connecting to: {}", ip);
    const auto is_connected = m_network_utils->connectSocket(*socket, ptrOrErrorCode.value().get());
    if (is_connected == StatusCode::FAIL)
    {
        LOG_ERROR(LOG_TAG, "connectSocket failed: {}", strerror(errno));
        return nullptr;
    }

    LOG_INFO(LOG_TAG, "Connected successfully ({})", ip);
    return std::make_unique<HttpConnection>(std::move(socket), m_network_utils);
}

} // namespace http