#include "http/HttpConnection.hpp"
#include "http/HttpHelpers.hpp"
#include "utils/network/INetworkUtils.hpp"
#include "utils/network/StatusCode.hpp"
#include "utils/network/Types.hpp"
#include "utils/network/TcpSocket.hpp"

#include <expected>
#include <string>
#include <cassert>

namespace {

using utils::network::TcpSocket;
using utils::network::INetworkUtils;
using utils::network::StatusCode;
using utils::network::BytesView;
using utils::network::AddrInfoPtr;

} // namespace

namespace http {

HttpConnection::HttpConnection(TcpSocket socket, std::shared_ptr<INetworkUtils> network_utils)
    : m_socket(std::move(socket))
    , m_network_utils(std::move(network_utils))
{
    assert(m_socket.isValid());
    assert(m_network_utils);
}

HttpConnection::HttpConnection(HttpConnection&& other) noexcept
{
    m_socket = std::move(other.m_socket);
    m_network_utils = std::move(other.m_network_utils);
}

HttpConnection&
HttpConnection::operator=(HttpConnection&& other) noexcept
{
    if (&other != this)
    {
        m_socket = std::move(other.m_socket);
        m_network_utils = std::move(other.m_network_utils);
    }

    return *this;
}

StatusCode
HttpConnection::sendData(std::string_view request_sv) const
{
    BytesView bytes{reinterpret_cast<const std::byte*>(request_sv.data()), request_sv.size()};
    return m_network_utils->sendBytes(m_socket, bytes);
}

std::expected<std::string, StatusCode>
HttpConnection::receiveData() const
{
    constexpr size_t buffer_size = 4096;
    std::vector<std::byte> received;
    received.reserve(buffer_size);

    // Read headers
    auto end_of_headers_pos = std::string::npos;
    do
    {
        auto bytes = m_network_utils->receiveBytes(m_socket);
        if (!bytes)
        {
            // Error occured
            return std::unexpected(bytes.error_or(StatusCode::FAIL));
        }

        received.insert(std::end(received), std::begin(*bytes), std::end(*bytes));
        end_of_headers_pos = findEndOfHeaders(
                std::string_view(reinterpret_cast<const char*>(received.data()), received.size()));
    } while (end_of_headers_pos == std::string::npos);

    std::string_view headers_sv(reinterpret_cast<const char*>(received.data()), end_of_headers_pos);
    auto content_length = getContentLength(headers_sv);
    if (!content_length)
    {
        // `Content-Length` header is missing
        return std::unexpected(StatusCode::NO_CONTENT_LENGTH);
    }

    // As we most probably read more than only headers, we need to calculate how much data left to
    // read in total. In order to calculate bytes which are still should be read, substract from
    // vector size header payload and 4 bytes of end of headers sequence.
    // (\r\n\r\n).
    size_t header_end = end_of_headers_pos + 4;
    size_t body_already_read = received.size() - header_end;
    size_t bytes_left =
            *content_length > body_already_read ? *content_length - body_already_read : 0;

    received.reserve(received.size() + bytes_left);
    // Read body
    while (bytes_left > 0)
    {
        auto bytes = m_network_utils->receiveBytes(m_socket);
        if (!bytes)
        {
            // Something unexpected happened, either peer closed the connection or error occured
            // during read.
            return std::unexpected(bytes.error_or(StatusCode::FAIL));
        }

        received.insert(std::end(received), std::begin(*bytes), std::end(*bytes));
        bytes_left = bytes_left > bytes->size() ? bytes_left - bytes->size() : 0;
    }

    std::string received_str(reinterpret_cast<const char*>(received.data()), received.size());
    return received_str;
}

bool
HttpConnection::isClosed() const
{
    return !m_socket.isValid();
}

void
HttpConnection::closeConnection()
{
    if (!isClosed())
    {
        m_socket.close();
    }
}

} // namespace http