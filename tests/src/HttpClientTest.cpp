#include "MockHttpClient.hpp"
#include "MockNetworkUtils.hpp"

#include <memory>

namespace {

using namespace testing;
using namespace http;

constexpr std::string TEST_IP = "www.example.com";
constexpr unsigned int TEST_PORT = 8080;
constexpr int TEST_SOCK_FD = 3;

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

TEST_F(HttpClientTest, when_create_connection_is_failed_then_nullptr_is_returned)
{
    EXPECT_CALL(*http_client, createConnectionImpl(_, _)).WillOnce(ReturnNull());
    const auto connection = http_client->createConnection(TEST_IP, TEST_PORT);
    ASSERT_EQ(connection, nullptr);
}

TEST_F(HttpClientTest, when_connection_created_successfully_then_it_shouldnot_be_closed)
{
    EXPECT_CALL(*http_client, createConnectionImpl(_, _))
            .WillOnce(Return(
                    std::move(std::make_unique<HttpConnection>(TEST_IP, TEST_PORT, TEST_SOCK_FD))));

    const auto connection = http_client->createConnection("www.example.com", 8080);
    ASSERT_NE(connection, nullptr);
    EXPECT_FALSE(connection->isClosed());

    EXPECT_CALL(*network_utils, closeSocket(_));
    http_client->closeConnection(*connection);
    EXPECT_TRUE(connection->isClosed());
}

TEST_F(HttpClientTest, when_2)
{
    // std::string ip;
    // unsigned int port;

    // EXPECT_CALL(*http_client, createConnectionImpl(_, _))
    //         .WillOnce(
    //                 DoAll(SaveArg<0>(&ip),
    //                       SaveArg<1>(&port),
    //                       Return(std::make_shared<HttpConnection>("1", 1, 1))));

    // const auto connection = http_client->createConnection("www.example.com", 8080);
    // ASSERT_NE(connection, nullptr);
    // EXPECT_FALSE(connection->isClosed());

    // connection->closeConnection();
    // EXPECT_TRUE(connection->isClosed());
}

} // namespace