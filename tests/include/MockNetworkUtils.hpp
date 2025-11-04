#pragma once

#include "utils/network/INetworkUtils.hpp"
#include "utils/network/Types.hpp"

#include <gmock/gmock.h>

namespace {

using utils::network::StatusCode;
using utils::network::AddrInfoPtr;
using utils::network::Socket;
using utils::network::Bytes;
using utils::network::BytesView;

struct MockNetworkUtils : public utils::network::INetworkUtils
{
    MOCK_METHOD(Socket, createSocket, (const addrinfo* addr), (override));

    MOCK_METHOD(StatusCode, connectSocket, (const Socket&, const addrinfo* info), (override));

    MOCK_METHOD(StatusCode, sendBytes, (const Socket&, BytesView), (override, const));

    MOCK_METHOD(std::optional<Bytes>, receiveBytes, (const Socket&), (override, const));

    MOCK_METHOD(
            (std::expected<AddrInfoPtr, int>),
            getAddrInfo,
            (std::string_view, std::string_view, const addrinfo*),
            (override));
};

} // namespace