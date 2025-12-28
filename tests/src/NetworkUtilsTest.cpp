#include "MockSysCallsWrapper.hpp"
#include "utils/network/NetworkUtils.hpp"
#include "TestHelpers.hpp"

namespace {

using namespace testing;
using utils::network::NetworkUtils;
using utils::network::AddrInfoPtr;

constexpr int INVALID_SOCK_FD = -1;
constexpr int VALID_SOCK_FD = 123;

struct NetworkUtilsTest : public Test
{
    NetworkUtilsTest()
    {
        syscalls_wrapper = std::make_shared<StrictMock<MockSysCallsWrapper>>();
        network_utils = std::make_unique<NetworkUtils>(syscalls_wrapper);
        valid_socket = createTcpSocketWithFd(VALID_SOCK_FD);
    }

    std::shared_ptr<StrictMock<MockSysCallsWrapper>> syscalls_wrapper;
    std::unique_ptr<NetworkUtils> network_utils;
    utils::network::TcpSocket valid_socket;
};

TEST_F(NetworkUtilsTest, when_addrinfo_is_nullptr_then_socket_syscall_is_never_called)
{
    EXPECT_CALL(*syscalls_wrapper, socketSyscall(_, _, _)).Times(0);
    auto socket = network_utils->createTcpSocket(nullptr);
    ASSERT_FALSE(socket);
}

TEST_F(NetworkUtilsTest,
       when_socket_syscall_repeatedly_fails_then_invalid_socket_descriptor_returned)
{
    // Setup addrinfo linked list
    const auto list_size = 10;
    auto* info = createAddrinfoList(list_size);
    ASSERT_NE(info, nullptr);

    // Test
    EXPECT_CALL(*syscalls_wrapper, socketSyscall(_, _, _))
            .Times(list_size)
            .WillRepeatedly(Return(-1));

    auto socket = network_utils->createTcpSocket(info);
    ASSERT_FALSE(socket);

    // Cleanup linked list
    deleteAddrinfoList(info);
}

TEST_F(NetworkUtilsTest, when_socket_syscall_successful_then_valid_socket_descriptor_returned)
{
    addrinfo* info = new addrinfo;
    ASSERT_NE(info, nullptr);

    EXPECT_CALL(*syscalls_wrapper, socketSyscall(_, _, _)).Times(1).WillOnce(Return(VALID_SOCK_FD));

    auto socket = network_utils->createTcpSocket(info);
    ASSERT_TRUE(socket);
    EXPECT_TRUE(socket->isValid());
    EXPECT_EQ(socket->fd(), VALID_SOCK_FD);

    delete info;
}

TEST_F(NetworkUtilsTest, when_socket_syscall_successful_then_other_addrinfo_entries_skipped)
{
    // Setup addrinfo linked list
    const auto list_size = 4;
    auto* info = createAddrinfoList(list_size);
    ASSERT_NE(info, nullptr);

    const auto expected_syscall_invokations = 2;
    EXPECT_CALL(*syscalls_wrapper, socketSyscall(_, _, _))
            .Times(expected_syscall_invokations)
            .WillOnce(Return(-1))
            .WillOnce(Return(VALID_SOCK_FD));

    auto socket = network_utils->createTcpSocket(info);
    ASSERT_TRUE(socket);
    EXPECT_TRUE(socket->isValid());
    EXPECT_EQ(socket->fd(), VALID_SOCK_FD);

    // Cleanup linked list
    deleteAddrinfoList(info);
}

TEST_F(NetworkUtilsTest, when_addrinfo_is_nullptr_then_connect_syscall_is_never_called)
{
    EXPECT_CALL(*syscalls_wrapper, connectSyscall(_, _, _)).Times(0);
    const auto status = network_utils->connectSocket(valid_socket, nullptr);
    ASSERT_EQ(status, utils::network::StatusCode::FAIL);
}

TEST_F(NetworkUtilsTest, when_connect_syscall_repeatedly_fails_then_fail_status_returned)
{
    // Setup addrinfo linked list
    const auto list_size = 10;
    auto* info = createAddrinfoList(list_size);
    ASSERT_NE(info, nullptr);

    EXPECT_CALL(*syscalls_wrapper, connectSyscall(_, _, _))
            .Times(list_size)
            .WillRepeatedly(Return(-1));

    const auto status = network_utils->connectSocket(valid_socket, info);
    EXPECT_EQ(status, utils::network::StatusCode::FAIL);

    // Cleanup linked list
    deleteAddrinfoList(info);
}

TEST_F(NetworkUtilsTest, when_connect_syscall_successful_then_status_code_is_ok)
{
    addrinfo* info = new addrinfo;
    ASSERT_NE(info, nullptr);

    EXPECT_CALL(*syscalls_wrapper, connectSyscall(_, _, _)).WillOnce(Return(0));

    const auto status = network_utils->connectSocket(valid_socket, info);
    EXPECT_EQ(status, utils::network::StatusCode::OK);

    delete info;
}

TEST_F(NetworkUtilsTest, when_connect_syscall_successful_then_other_addrinfo_entries_skipped)
{
    // Setup addrinfo linked list
    const auto list_size = 4;
    auto* info = createAddrinfoList(list_size);
    ASSERT_NE(info, nullptr);

    const auto expected_syscall_invokations = 2;
    EXPECT_CALL(*syscalls_wrapper, connectSyscall(_, _, _))
            .Times(expected_syscall_invokations)
            .WillOnce(Return(-1))
            .WillOnce(Return(0));

    const auto status = network_utils->connectSocket(valid_socket, info);
    EXPECT_EQ(status, utils::network::StatusCode::OK);

    // Cleanup linked list
    deleteAddrinfoList(info);
}

TEST_F(NetworkUtilsTest, when_getaddrinfo_syscall_fails_then_nullptr_and_error_code_returned)
{
    const int error_code = EAI_FAIL;

    EXPECT_CALL(*syscalls_wrapper, getaddrinfoSyscall(_, _, _, _)).WillOnce(Return(error_code));
    const auto ptrOrError = network_utils->getAddrInfo("ip", "port", nullptr);

    EXPECT_FALSE(ptrOrError.has_value());
    EXPECT_EQ(ptrOrError.error(), error_code);
}

TEST_F(NetworkUtilsTest, when_getaddrinfo_syscall_successful_then_valid_data_returned)
{
    addrinfo* info = (addrinfo*) malloc(sizeof(addrinfo));
    std::memset(info, 0, sizeof(addrinfo));

    EXPECT_CALL(*syscalls_wrapper, getaddrinfoSyscall(_, _, _, _))
            .WillOnce(DoAll(SetArgumentPointee<3>(info), Return(0)));

    const auto ptrOrError = network_utils->getAddrInfo("ip", "port", nullptr);
    EXPECT_TRUE(ptrOrError.has_value());
}

} // namespace