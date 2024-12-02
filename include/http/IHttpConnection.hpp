#pragma once

#include "utils/network/StatusCode.hpp"

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
    sendBytes(std::stringstream& bytes) const = 0;

    virtual std::stringstream
    receiveBytes() const = 0;

protected:
    IHttpConnection() = default;
};

} // namespace http