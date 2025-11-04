#pragma once

#include "utils/network/INetworkUtils.hpp"

#include <cstring>

namespace {
using utils::network::Socket;

Socket
createSocketWithFd(const int fd)
{
    int* sock_fd = new int;
    if (sock_fd == nullptr)
    {
        return nullptr;
    }

    *sock_fd = fd;
    Socket socket(sock_fd, [](int* sock_fd) -> int {
        if (sock_fd != nullptr)
        {
            delete sock_fd;
        }

        return 0;
    });

    return socket;
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