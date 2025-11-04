#pragma once

#include "utils/network/StatusCode.hpp"
#include "utils/network/Types.hpp"

#include <sstream>

namespace http {

class IHttpConnection
{
public:
    virtual ~IHttpConnection() = default;

    IHttpConnection(const IHttpConnection&) = delete;

    IHttpConnection&
    operator=(const IHttpConnection&) = delete;

    virtual utils::network::StatusCode
    sendBytes(utils::network::BytesView bytes) const = 0;

    virtual std::optional<utils::network::Bytes>
    receiveBytes() const = 0;

protected:
    IHttpConnection() = default;
};

} // namespace http