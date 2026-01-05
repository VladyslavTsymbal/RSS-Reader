#pragma once

#include "utils/network/INetworkUtils.hpp"
#include "utils/network/Types.hpp"

#include <gmock/gmock.h>

namespace {

using utils::network::StatusCode;
using utils::network::AddrInfoPtr;
using utils::network::TcpSocket;
using utils::network::Bytes;
using utils::network::BytesView;
using utils::network::Port;

struct MockNetworkUtils : public utils::network::INetworkUtils
{
    MOCK_METHOD(std::optional<TcpSocket>, createTcpSocket, (const AddrInfoPtr& addr), (override));

    MOCK_METHOD(StatusCode, connectSocket, (const TcpSocket&, const AddrInfoPtr& info), (override));

    MOCK_METHOD(
            std::optional<TcpSocket>,
            acceptSocket,
            (const TcpSocket&, const AddrInfoPtr&),
            (override, const));

    MOCK_METHOD(StatusCode, sendBytes, (const TcpSocket&, BytesView), (override, const));

    MOCK_METHOD(
            (std::expected<Bytes, StatusCode>),
            receiveBytes,
            (const TcpSocket&),
            (override, const));

    MOCK_METHOD(
            (std::expected<AddrInfoPtr, int>),
            getAddrInfo,
            (std::string_view, Port port, const AddrInfoPtr&),
            (override));
};

} // namespace