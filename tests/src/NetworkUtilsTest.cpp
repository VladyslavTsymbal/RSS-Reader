#include "MockSysCallsWrapper.hpp"
#include "utils/network/NetworkUtils.hpp"

#include <cstring>

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
    }

    std::shared_ptr<StrictMock<MockSysCallsWrapper>> syscalls_wrapper;
    std::unique_ptr<NetworkUtils> network_utils;
};

addrinfo*
createAddrinfoList(const size_t size)
{
    addrinfo* head = new addrinfo;
    std::memset(head, 0, sizeof(addrinfo));
    addrinfo* temp = head;

    for (auto i = 0; i < size - 1; ++i)
    {
        temp->ai_next = new addrinfo;
        temp = temp->ai_next;
        std::memset(temp, 0, sizeof(addrinfo));
    }

    return head;
}

void
deleteAddrinfoList(addrinfo* list)
{
    if (list == nullptr)
    {
        return;
    }

    addrinfo* head = list;
    addrinfo* next = head->ai_next;

    while (next != nullptr)
    {
        delete head;
        head = next;
        next = head->ai_next;
    }

    if (head)
    {
        delete head;
    }
}

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

TEST_F(NetworkUtilsTest, when_addrinfo_is_nullptr_then_socket_syscall_is_never_called)
{
    EXPECT_CALL(*syscalls_wrapper, socketSyscall(_, _, _)).Times(0);
    const int sock_fd = network_utils->createSocket(nullptr);
    ASSERT_EQ(sock_fd, -1);
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

    const int sock_fd = network_utils->createSocket(info);
    EXPECT_EQ(sock_fd, -1);

    // Cleanup linked list
    deleteAddrinfoList(info);
}

TEST_F(NetworkUtilsTest, when_socket_syscall_successful_then_valid_socket_descriptor_returned)
{
    addrinfo* info = new addrinfo;
    ASSERT_NE(info, nullptr);

    EXPECT_CALL(*syscalls_wrapper, socketSyscall(_, _, _)).Times(1).WillOnce(Return(VALID_SOCK_FD));

    const int sock_fd = network_utils->createSocket(info);
    EXPECT_EQ(sock_fd, VALID_SOCK_FD);

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

    const int sock_fd = network_utils->createSocket(info);
    EXPECT_EQ(sock_fd, VALID_SOCK_FD);

    // Cleanup linked list
    deleteAddrinfoList(info);
}

TEST_F(NetworkUtilsTest, when_addrinfo_is_nullptr_then_connect_syscall_is_never_called)
{
    EXPECT_CALL(*syscalls_wrapper, connectSyscall(_, _, _)).Times(0);
    const auto status = network_utils->connectSocket(VALID_SOCK_FD, nullptr);
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

    const auto status = network_utils->connectSocket(VALID_SOCK_FD, info);
    EXPECT_EQ(status, utils::network::StatusCode::FAIL);

    // Cleanup linked list
    deleteAddrinfoList(info);
}

TEST_F(NetworkUtilsTest, when_connect_syscall_successful_then_status_code_is_ok)
{
    addrinfo* info = new addrinfo;
    ASSERT_NE(info, nullptr);

    EXPECT_CALL(*syscalls_wrapper, connectSyscall(_, _, _)).WillOnce(Return(0));

    const auto status = network_utils->connectSocket(VALID_SOCK_FD, info);
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

    const auto status = network_utils->connectSocket(VALID_SOCK_FD, info);
    EXPECT_EQ(status, utils::network::StatusCode::OK);

    // Cleanup linked list
    deleteAddrinfoList(info);
}

TEST_F(NetworkUtilsTest, when_getaddrinfo_syscall_fails_then_nullptr_and_error_code_returned)
{
    const int error_code = EAI_FAIL;

    EXPECT_CALL(*syscalls_wrapper, getaddrinfoSyscall(_, _, _, _)).WillOnce(Return(error_code));
    const auto [result, error] = network_utils->getAddrInfo("ip", "port", nullptr);

    EXPECT_EQ(result, nullptr);
    EXPECT_EQ(error, error_code);
}

TEST_F(NetworkUtilsTest, when_getaddrinfo_syscall_successful_then_valid_data_returned)
{
    addrinfo* info = (addrinfo*) malloc(sizeof(addrinfo));
    std::memset(info, 0, sizeof(addrinfo));

    EXPECT_CALL(*syscalls_wrapper, getaddrinfoSyscall(_, _, _, _))
            .WillOnce(DoAll(SetArgumentPointee<3>(info), Return(0)));
    const auto [result, error] = network_utils->getAddrInfo("ip", "port", nullptr);

    EXPECT_NE(result, nullptr);
    EXPECT_EQ(error, 0);
}

} // namespace