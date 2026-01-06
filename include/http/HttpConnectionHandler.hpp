#pragma once

#include "http/HttpConnectionState.hpp"
#include "network/StatusCode.hpp"
#include "network/TcpSocket.hpp"
#include "network/Types.hpp"

namespace network {
class TcpConnection;
} // namespace network

namespace http {

class HttpRequest;

class HttpConnectionHandler
{
public:
    HttpConnectionHandler() = delete;

    HttpConnectionHandler(std::unique_ptr<network::TcpConnection> connection);

    HttpConnectionHandler(HttpConnectionHandler&& other) noexcept;

    HttpConnectionHandler&
    operator=(HttpConnectionHandler&& other) noexcept;

    network::StatusCode
    readAvailable();

    network::StatusCode
    writeAvailable();

    HttpConnectionState
    getState() const;

    // TODO: Just for testing, remove later
    void
    processData();

private:
    struct ConnectionBuffers
    {
        network::Bytes in;
        network::Bytes out;

        size_t in_header_size{};
        size_t in_payload_size{};

        size_t out_offset{};
        size_t out_size{};
    };

    bool
    checkHeadersReceived();

    bool
    checkPayloadReceived() const;

    bool
    writeFinished() const;

    // TODO: Remove later, just for testing now
    std::string
    createResponse(const HttpRequest& request) const;

    network::TcpSocket m_socket;
    std::unique_ptr<network::TcpConnection> m_connection{nullptr};
    std::unique_ptr<ConnectionBuffers> m_buffers{nullptr};
    HttpConnectionState m_state{HttpConnectionState::RECEIVING_HEADERS};
    bool m_are_headers_received{false};
    bool m_is_payload_present{false};
    bool m_is_payload_received{false};
};

} // namespace http