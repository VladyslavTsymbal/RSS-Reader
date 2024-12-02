#include "utils/network/ISysCallsWrapper.hpp"

#include <gmock/gmock.h>

namespace {

struct MockSysCallsWrapper : public utils::network::ISysCallsWrapper
{
    MOCK_METHOD(int, socketSyscall, (int, int, int), (override));

    MOCK_METHOD(int, connectSyscall, (int, const sockaddr*, socklen_t), (override));

    MOCK_METHOD(
            int,
            getaddrinfoSyscall,
            (const char*, const char*, const addrinfo*, addrinfo**),
            (override));
};

} // namespace