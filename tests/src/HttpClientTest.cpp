#include "MockHttpClient.hpp"
#include "MockNetworkUtils.hpp"

#include <gmock/gmock.h>

namespace {

using namespace testing;
using namespace http;
using utils::network::AddrInfoPtr;

constexpr std::string TEST_IP = "www.example.com";
constexpr unsigned int TEST_PORT = 8080;
constexpr int TEST_SOCK_FD = 3;
constexpr int VALID_SOCKET_FD = 10;

struct HttpClientTest : public Test
{
    void
    SetUp()
    {
        network_utils = std::make_shared<StrictMock<MockNetworkUtils>>();
        http_client = std::make_unique<StrictMock<MockHttpClient>>(network_utils);
    }

    std::unique_ptr<StrictMock<MockHttpClient>> http_client;
    std::shared_ptr<StrictMock<MockNetworkUtils>> network_utils;
};

TEST_F(HttpClientTest, when_addrinfo_returns_nullptr_then_connection_is_nullptr)
{
    EXPECT_CALL(*network_utils, getAddrInfo(_, _, _))
            .WillOnce(Return(std::make_pair<AddrInfoPtr, int>(nullptr, EAI_FAIL)));

    const auto connection = http_client->createConnection(TEST_IP, TEST_PORT);
    ASSERT_EQ(connection, nullptr);
}

TEST_F(HttpClientTest, when_createSocket_fails_then_connection_is_nullptr)
{
    auto addr_info = std::make_pair<AddrInfoPtr, int>(std::make_unique<addrinfo>(), 0);
    EXPECT_CALL(*network_utils, getAddrInfo(_, _, _)).WillOnce(Return(std::move(addr_info)));
    EXPECT_CALL(*network_utils, createSocket(_)).WillOnce(Return(-1));

    const auto connection = http_client->createConnection(TEST_IP, TEST_PORT);
    ASSERT_EQ(connection, nullptr);
}

TEST_F(HttpClientTest, when_connectSocket_fails_then_connection_is_nullptr)
{
    auto addr_info = std::make_pair<AddrInfoPtr, int>(std::make_unique<addrinfo>(), 0);
    EXPECT_CALL(*network_utils, getAddrInfo(_, _, _)).WillOnce(Return(std::move(addr_info)));
    EXPECT_CALL(*network_utils, createSocket(_)).WillOnce(Return(VALID_SOCKET_FD));
    EXPECT_CALL(*network_utils, connectSocket(_, _)).WillOnce(Return(StatusCode::FAIL));
    EXPECT_CALL(*network_utils, closeSocket(_));

    const auto connection = http_client->createConnection(TEST_IP, TEST_PORT);
    ASSERT_EQ(connection, nullptr);
}

TEST_F(HttpClientTest, when_connectSocket_is_succeed_then_valid_connection_returned)
{
    auto addr_info = std::make_pair<AddrInfoPtr, int>(std::make_unique<addrinfo>(), 0);
    EXPECT_CALL(*network_utils, getAddrInfo(_, _, _)).WillOnce(Return(std::move(addr_info)));
    EXPECT_CALL(*network_utils, createSocket(_)).WillOnce(Return(VALID_SOCKET_FD));
    EXPECT_CALL(*network_utils, connectSocket(_, _)).WillOnce(Return(StatusCode::OK));

    const auto connection = http_client->createConnection(TEST_IP, TEST_PORT);
    ASSERT_NE(connection, nullptr);
    EXPECT_FALSE(connection->isClosed());
}

TEST_F(HttpClientTest, when_http_client_closes_connection_then_connection_is_closed)
{
    auto addr_info = std::make_pair<AddrInfoPtr, int>(std::make_unique<addrinfo>(), 0);
    EXPECT_CALL(*network_utils, getAddrInfo(_, _, _)).WillOnce(Return(std::move(addr_info)));
    EXPECT_CALL(*network_utils, createSocket(_)).WillOnce(Return(VALID_SOCKET_FD));
    EXPECT_CALL(*network_utils, connectSocket(_, _)).WillOnce(Return(StatusCode::OK));

    const auto connection = http_client->createConnection(TEST_IP, TEST_PORT);
    ASSERT_NE(connection, nullptr);
    EXPECT_FALSE(connection->isClosed());

    EXPECT_CALL(*network_utils, closeSocket(_));
    http_client->closeConnection(*connection);
    EXPECT_TRUE(connection->isClosed());
}

TEST_F(HttpClientTest,
       when_connection_is_repeateadly_closed_by_client_then_close_socket_called_once)
{
    auto addr_info = std::make_pair<AddrInfoPtr, int>(std::make_unique<addrinfo>(), 0);
    EXPECT_CALL(*network_utils, getAddrInfo(_, _, _)).WillOnce(Return(std::move(addr_info)));

    EXPECT_CALL(*network_utils, createSocket(_)).WillOnce(Return(VALID_SOCKET_FD));
    EXPECT_CALL(*network_utils, connectSocket(_, _)).WillOnce(Return(StatusCode::OK));

    const auto connection = http_client->createConnection(TEST_IP, TEST_PORT);
    ASSERT_NE(connection, nullptr);
    EXPECT_FALSE(connection->isClosed());

    EXPECT_CALL(*network_utils, closeSocket(_));
    http_client->closeConnection(*connection);
    EXPECT_TRUE(connection->isClosed());

    http_client->closeConnection(*connection);
    http_client->closeConnection(*connection);
}

} // namespace