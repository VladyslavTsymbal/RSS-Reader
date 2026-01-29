#pragma once

#include <sys/poll.h>
#include <variant>

namespace http {

struct HandlerInterest
{
    short int poll_interest;
};

struct ReadyToRead
{
    HandlerInterest interest{POLLIN | POLLHUP};
};

struct Receiving
{
    HandlerInterest interest{POLLIN | POLLHUP};
};

struct Processing
{
    HandlerInterest interest{POLLOUT | POLLHUP};
};

struct Writing
{
    HandlerInterest interest{POLLOUT | POLLHUP};
};

struct WaitingToWrite
{
    HandlerInterest interest{POLLOUT | POLLHUP};
};

struct Closing
{
    HandlerInterest interest{0};
};

using HandlerState =
        std::variant<ReadyToRead, Receiving, Writing, WaitingToWrite, Processing, Closing>;

} // namespace http
