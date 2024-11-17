#pragma once

#include <string>

namespace http {

class IHttpConnection
{
public:
    virtual ~IHttpConnection() = default;

    IHttpConnection(const IHttpConnection&) = delete;
    IHttpConnection&
    operator=(const IHttpConnection&) = delete;

    virtual bool
    isClosed() const = 0;

    virtual void
    closeConnection() = 0;

    virtual std::string
    getUrl() const = 0;

    virtual int
    getSocket() const = 0;

    virtual unsigned int
    getPort() const = 0;

protected:
    IHttpConnection() = default;
};

} // namespace http