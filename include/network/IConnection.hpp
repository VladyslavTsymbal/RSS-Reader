#pragma once

#include "INonCopyable.hpp"
#include "network/IClosable.hpp"
#include "network/StatusCode.hpp"
#include "network/Types.hpp"

#include <expected>

namespace network {

struct IConnection : public IClosable, public INonCopyable
{
    virtual std::expected<int, network::StatusCode>
    sendBytes(network::BytesView bytes) const = 0;

    virtual std::expected<network::Bytes, network::StatusCode>
    receiveBytes(const size_t buffer_size) const = 0;

    virtual ~IConnection() = default;
};

} // namespace network