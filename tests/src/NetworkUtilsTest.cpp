#include "MockSysCallsWrapper.hpp"
#include "utils/NetworkUtils.hpp"

namespace {

using namespace testing;
using utils::network::NetworkUtils;

constexpr int INVALID_SOCK_FD = -1;
constexpr int VALID_SOCK_FD = 123;

struct NetworkUtilsTest : public Test
{
    NetworkUtilsTest()
    {
        syscalls_wrapper = std::make_shared<StrictMock<MockSysCallsWrapper>>();
        network_utils = std::make_unique<NetworkUtils>(syscalls_wrapper);
    }

    std::shared_ptr<StrictMock<MockSysCallsWrapper>> syscalls_wrapper;
    std::unique_ptr<NetworkUtils> network_utils;
};

TEST_F(NetworkUtilsTest, when_valid_fd_passed_closeSocket_then_close_syscall_invoked)
{
    EXPECT_CALL(*syscalls_wrapper, closeSyscall(_));
    network_utils->closeSocket(VALID_SOCK_FD);
}

TEST_F(NetworkUtilsTest, when_invalid_fd_passed_closeSocket_then_close_syscall_not_invoked)
{
    EXPECT_CALL(*syscalls_wrapper, closeSyscall(_)).Times(0);
    network_utils->closeSocket(INVALID_SOCK_FD);
}

} // namespace