#pragma once

#include "utils/network/INetworkUtils.hpp"

#include <cstring>

namespace {

using utils::network::TcpSocket;
using utils::network::Socket;

TcpSocket
createTcpSocketWithFd(const int fd)
{
    return TcpSocket(Socket(fd));
}

addrinfo*
createAddrinfoList(const size_t size)
{
    addrinfo* head = new addrinfo;
    std::memset(head, 0, sizeof(addrinfo));
    addrinfo* temp = head;

    for (auto i = 0; i < size - 1; ++i)
    {
        temp->ai_next = new addrinfo;
        temp = temp->ai_next;
        std::memset(temp, 0, sizeof(addrinfo));
    }

    return head;
}

void
deleteAddrinfoList(addrinfo* list)
{
    if (list == nullptr)
    {
        return;
    }

    addrinfo* head = list;
    addrinfo* next = head->ai_next;

    while (next != nullptr)
    {
        delete head;
        head = next;
        next = head->ai_next;
    }

    if (head)
    {
        delete head;
    }
}

} // namespace