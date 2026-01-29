#pragma once

#include "network/Types.hpp"

#include <variant>

namespace network::event {

struct Accept
{
};

struct Read
{
    const Fd fd;
};

struct Write
{
    const Fd fd;
};

struct Close
{
    const Fd fd;
};

using PollEvent = std::variant<Accept, Read, Write, Close>;

} // namespace network::event
