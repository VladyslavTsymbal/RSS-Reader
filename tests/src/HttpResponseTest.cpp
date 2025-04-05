#include "http/HttpResponse.hpp"

#include <gtest/gtest.h>
#include <optional>
#include <string_view>
#include <sstream>

namespace {

const std::string_view okay_response =
        "HTTP/1.1 200 OK\r\n"
        "Date: Tue, 02 Apr 2024 12:00:00 GMT\r\n"
        "Server: ExampleServer/1.0\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "Content-Length: 88\r\n"
        "Connection: keep-alive\r\n"
        "\r\n"
        "<!doctype html>"
        "<html>"
        "<head><title>success</title></head>"
        "<body><h1>it works!</h1></body>"
        "</html>";

const std::string_view forbidden_response =
        "HTTP/1.1 403 Forbidden\r\n"
        "Date: Tue, 02 Apr 2024 12:00:00 GMT\r\n"
        "Server: ExampleServer/1.0\r\n"
        "Connection: close\r\n"
        "\r\n"
        "{"
        "\"error\": \"Forbidden\","
        "\"message\": \"You do not have permission to access this resource.\""
        "}";

const std::string_view no_content_response =
        "HTTP/1.1 204 No Content\r\n"
        "Date: Tue, 02 Apr 2024 12:00:00 GMT\r\n"
        "Server: ExampleServer/1.0\r\n"
        "Connection: close\r\n"
        "\r\n";

TEST(HttpResponseTest, when_response_has_okay_status_code_then_response_is_successful)
{
    std::stringstream ss;
    ss << okay_response;
    http::HttpResponse response(std::move(ss));

    EXPECT_TRUE(response.isSuccessful());
    EXPECT_EQ(response.getStatusCode(), 200);
}

TEST(HttpResponseTest, when_response_has_not_okay_status_code_then_response_is_not_successful)
{
    std::stringstream ss;
    ss << forbidden_response;
    http::HttpResponse response(std::move(ss));

    EXPECT_FALSE(response.isSuccessful());
    EXPECT_EQ(response.getStatusCode(), 403);
}

TEST(HttpResponseTest, when_present_header_data_asked_by_key_then_data_returned)
{
    std::stringstream ss;
    ss << forbidden_response;
    http::HttpResponse response(std::move(ss));

    EXPECT_NE(response.getHeader("Date"), std::nullopt);
    EXPECT_EQ(response.getHeader("Date"), "Tue, 02 Apr 2024 12:00:00 GMT");
}

TEST(HttpResponseTest, when_not_present_header_data_asked_by_key_then_nullopt_returned)
{
    std::stringstream ss;
    ss << forbidden_response;
    http::HttpResponse response(std::move(ss));

    EXPECT_EQ(response.getHeader("Content-Type"), std::nullopt);
}

TEST(HttpResponseTest, when_get_description_called_then_actual_description_returned)
{
    std::stringstream ss;
    ss << no_content_response;
    http::HttpResponse response(std::move(ss));

    EXPECT_EQ(response.getDescription(), "No Content");
}

TEST(HttpResponseTest, when_response_has_no_body_then_get_body_returns_empty_string)
{
    std::stringstream ss;
    ss << no_content_response;
    http::HttpResponse response(std::move(ss));

    EXPECT_TRUE(response.getBody().empty());
}

} // namespace