#pragma once

namespace network {

struct IClosable
{
    virtual bool isClosed() const = 0;
    virtual void close() = 0;
};

} // namespace network
