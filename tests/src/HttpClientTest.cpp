#include <gmock/gmock.h>
#include <memory>

#include "http/HttpClient.hpp"
#include "utils/StatusCode.hpp"

namespace {

using namespace testing;
using namespace http;

using HttpConnection = HttpClient::HttpConnection;

struct MockHttpClient final : public HttpClient
{
    MOCK_METHOD(
            utils::network::StatusCode,
            sendRequestImpl,
            (const int socket_fd, const std::string&),
            (override));

    MOCK_METHOD(
            std::optional<HttpConnection>,
            createConnectionImpl,
            (std::string ip, const unsigned int port),
            (override));

    MOCK_METHOD(std::stringstream, getResponseImpl, (const int), (override));
};

struct HttpClientTest : public Test
{
    std::unique_ptr<MockHttpClient> http_client = std::make_unique<MockHttpClient>();
};

TEST_F(HttpClientTest, when_create_connection_is_failed_then_nullopt_is_returned)
{
    EXPECT_CALL(*http_client, createConnectionImpl(_, _)).WillOnce(Return(std::nullopt));
    const auto connection = http_client->createConnection("www.example.com", 8080);
    ASSERT_EQ(connection, std::nullopt);
}

// TEST_F(HttpClientTest, when_closed_connection_passed_to_send_request_then_nullopt_is_returned)
// {
//     EXPECT_CALL(*http_client, createConnectionImpl(_, _)).WillOnce(Return(HttpConnection{}));
//     const auto connection = http_client->createConnection("www.example.com", 8080);
//     ASSERT_EQ(connection, std::nullopt);
// }

} // namespace