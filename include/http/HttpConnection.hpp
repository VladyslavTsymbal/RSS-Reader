#pragma once

#include "http/IHttpConnection.hpp"

#include <string>

namespace http {

class HttpClient;

class HttpConnection : public IHttpConnection
{
public:
    friend class HttpClient;

    // Should be used only by HttpClient
    HttpConnection(std::string ip, const unsigned int port, const int socket);
    ~HttpConnection() override;

    HttpConnection(HttpConnection&& other) noexcept;

    HttpConnection&
    operator=(HttpConnection&& other) noexcept;

    bool
    isClosed() const override;

    void
    closeConnection() override;

    std::string
    getUrl() const override;

    int
    getSocket() const override;

    unsigned int
    getPort() const override;

private:
    std::string m_ip_address;
    unsigned int m_port;
    // TODO: Socket should support RAII idiom
    int m_sock_fd;
    bool m_is_closed{false};
};

} // namespace http