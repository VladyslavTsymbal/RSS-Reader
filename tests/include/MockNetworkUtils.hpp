#pragma once

#include "utils/network/INetworkUtils.hpp"

#include <gmock/gmock.h>

namespace {

using utils::network::StatusCode;
using utils::network::AddrInfoPtr;

struct MockNetworkUtils : public utils::network::INetworkUtils
{
    MOCK_METHOD(int, createSocket, (const addrinfo* addr), (override));

    MOCK_METHOD(void, closeSocket, (const int), (override));

    MOCK_METHOD(StatusCode, connectSocket, (const int, const addrinfo* info), (override));

    MOCK_METHOD(
            (std::pair<AddrInfoPtr, int>),
            getAddrInfo,
            (const std::string&, const std::string&, const addrinfo*),
            (override));
};

} // namespace