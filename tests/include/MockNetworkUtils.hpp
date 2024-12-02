#pragma once

#include "utils/network/INetworkUtils.hpp"

#include <gmock/gmock.h>

namespace {

using utils::network::StatusCode;
using utils::network::AddrInfoPtr;
using utils::network::Socket;

struct MockNetworkUtils : public utils::network::INetworkUtils
{
    MOCK_METHOD(Socket, createSocket, (const addrinfo* addr), (override));

    MOCK_METHOD(StatusCode, connectSocket, (const int, const addrinfo* info), (override));

    MOCK_METHOD(StatusCode, sendBytes, (const int, std::istream&), (override, const));

    MOCK_METHOD(std::stringstream, receiveBytes, (const int), (override, const));

    MOCK_METHOD(
            (std::expected<AddrInfoPtr, int>),
            getAddrInfo,
            (std::string_view, std::string_view, const addrinfo*),
            (override));
};

} // namespace