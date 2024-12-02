#include "MockNetworkUtils.hpp"
#include "http/HttpConnectionFactory.hpp"
#include "http/IHttpConnection.hpp"

namespace {

using namespace testing;
using namespace http;

constexpr std::string_view TEST_IP = "ip";
constexpr unsigned int TEST_PORT = 123;
constexpr int TEST_SOCK_FD = 3;
constexpr int VALID_SOCKET_FD = 10;

struct HttpConnectionFactoryTest : public Test
{
    void
    SetUp()
    {
        network_utils = std::make_shared<StrictMock<MockNetworkUtils>>();
        connection_factory = std::make_unique<HttpConnectionFactory>(network_utils);
    }

    std::unique_ptr<HttpConnectionFactory> connection_factory;
    std::shared_ptr<StrictMock<MockNetworkUtils>> network_utils;
};

Socket
createSocketWithFd(const int fd)
{
    int* sock_fd = new int;
    if (sock_fd == nullptr)
    {
        return nullptr;
    }

    *sock_fd = fd;
    Socket socket(sock_fd, [](int* sock_fd) -> int {
        if (sock_fd != nullptr)
        {
            delete sock_fd;
        }

        return 0;
    });

    return socket;
}

TEST_F(HttpConnectionFactoryTest, when_addrinfo_returns_nullptr_then_connection_is_nullptr)
{
    EXPECT_CALL(*network_utils, getAddrInfo(_, _, _)).WillOnce(Return(std::unexpected(EAI_FAIL)));

    const auto connection = connection_factory->createConnection(TEST_IP, TEST_PORT);
    ASSERT_EQ(connection, nullptr);
}

TEST_F(HttpConnectionFactoryTest, when_createSocket_fails_then_connection_is_nullptr)
{
    auto addr_info = std::make_unique<addrinfo>();
    EXPECT_CALL(*network_utils, getAddrInfo(_, _, _)).WillOnce(Return(std::move(addr_info)));
    EXPECT_CALL(*network_utils, createSocket(_)).WillOnce(ReturnNull());

    const auto connection = connection_factory->createConnection(TEST_IP, TEST_PORT);
    ASSERT_EQ(connection, nullptr);
}

TEST_F(HttpConnectionFactoryTest, when_connectSocket_fails_then_connection_is_nullptr)
{
    auto addr_info = std::make_unique<addrinfo>();
    Socket socket = createSocketWithFd(VALID_SOCKET_FD);
    ASSERT_NE(socket, nullptr);

    EXPECT_CALL(*network_utils, getAddrInfo(_, _, _)).WillOnce(Return(std::move(addr_info)));
    EXPECT_CALL(*network_utils, createSocket(_)).WillOnce(Return(std::move(socket)));
    EXPECT_CALL(*network_utils, connectSocket(_, _)).WillOnce(Return(StatusCode::FAIL));

    const auto connection = connection_factory->createConnection(TEST_IP, TEST_PORT);
    ASSERT_EQ(connection, nullptr);
}

TEST_F(HttpConnectionFactoryTest, when_connectSocket_is_succeed_then_valid_connection_returned)
{
    auto addr_info = std::make_unique<addrinfo>();
    Socket socket = createSocketWithFd(VALID_SOCKET_FD);
    ASSERT_NE(socket, nullptr);

    EXPECT_CALL(*network_utils, getAddrInfo(_, _, _)).WillOnce(Return(std::move(addr_info)));
    EXPECT_CALL(*network_utils, createSocket(_)).WillOnce(Return(std::move(socket)));
    EXPECT_CALL(*network_utils, connectSocket(_, _)).WillOnce(Return(StatusCode::OK));

    const auto connection = connection_factory->createConnection(TEST_IP, TEST_PORT);
    ASSERT_NE(connection, nullptr);
}

} // namespace