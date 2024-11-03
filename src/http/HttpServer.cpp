#include "http/HttpServer.hpp"
#include "utils/NetworkUtils.hpp"

#include <arpa/inet.h>

#include <iostream>

namespace http {

// HttpServer

HttpServer::HttpServer(std::string port)
    : m_port(std::move(port))
{
    init();
}

HttpServer::~HttpServer()
{
    utils::network::closeSocket(m_sock_fd);
}

void
HttpServer::init()
{
    // using utils::network::AddrInfoBuilder;
    // using utils::network::AddrInfoPtr;

    // const auto hints = AddrInfoBuilder()
    //     .setProtocolFamily(AddrInfoBuilder::ProtocolFamily::UNSPECIFIED)
    //     .setSockType(AddrInfoBuilder::SockType::TCP)
    //     .setFlags(AI_PASSIVE)
    //     .build();

    // AddrInfoPtr result = utils::network::getAddrInfo(nullptr, m_port, &hints);
    // m_sock_fd = utils::network::createSocket(result.get());

    // char buf_ip[INET6_ADDRSTRLEN];
    // in_port_t port;
    // void* addr;
    // if (result->ai_family == AF_INET)
    // {
    //     struct sockaddr_in* sock_addr = (struct sockaddr_in*)result->ai_addr;
    //     addr = &(sock_addr->sin_addr);
    //     port = sock_addr->sin_port;
    // }
    // else
    // {
    //     struct sockaddr_in6* sock_addr = (struct sockaddr_in6*)result->ai_addr;
    //     addr = &(sock_addr->sin6_addr);
    //     port = sock_addr->sin6_port;
    // }

    // inet_ntop(result->ai_family, addr, buf_ip, sizeof(buf_ip));
    // std::cout << "Server created (" << buf_ip << ":" << ntohs(port) << ")\n";
}

} // namespace http