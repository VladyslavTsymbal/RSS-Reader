#pragma once

#include "utils/network/StatusCode.hpp"

#include <expected>
#include <string_view>

namespace http {

class HttpRequest;

class IHttpConnection
{
public:
    virtual ~IHttpConnection() = default;

    IHttpConnection(const IHttpConnection&) = delete;

    IHttpConnection&
    operator=(const IHttpConnection&) = delete;

    virtual utils::network::StatusCode sendData(std::string_view) const = 0;

    virtual std::expected<std::string, utils::network::StatusCode>
    receiveData() const = 0;

    virtual bool
    isClosed() const = 0;

    virtual void
    closeConnection() = 0;

protected:
    IHttpConnection() = default;
};

} // namespace http