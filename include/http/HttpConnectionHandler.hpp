#pragma once

#include "http/HttpConnectionState.hpp"

#include "network/PollEvents.hpp"

namespace network {
class TcpConnection;
} // namespace network

namespace http {

struct HandlerStateVisitor;
struct ConnectionBuffers;

class HttpConnectionHandler
{
public:
    HttpConnectionHandler() = delete;
    ~HttpConnectionHandler();

    HttpConnectionHandler(std::unique_ptr<network::TcpConnection> connection);

    HttpConnectionHandler(HttpConnectionHandler&& other) noexcept;

    HttpConnectionHandler&
    operator=(HttpConnectionHandler&& other) noexcept;

    HandlerInterest
    handleEvent(network::event::PollEvent event);

private:
    friend struct HandlerStateVisitor;

    std::unique_ptr<network::TcpConnection> m_connection{nullptr};
    std::unique_ptr<ConnectionBuffers> m_buffers{nullptr};
    HandlerState m_state;
};

} // namespace http
