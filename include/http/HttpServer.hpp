#pragma once

#include <string>

namespace http {

class HttpServer
{
public:
    HttpServer(std::string port);
    ~HttpServer();

private:
    void
    init();

private:
    const std::string m_port;
    int m_sock_fd{0};
};

} // namespace http