#include "http/HttpResponse.hpp"

#include <gtest/gtest.h>
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

TEST(HttpResponseTest, when_response_has_okay_status_code_then_response_is_successful)
{
    std::stringstream ss;
    ss << okay_response;
    http::HttpResponse response(std::move(ss));

    EXPECT_TRUE(response.isSuccessful());
}

TEST(HttpResponseTest, when_response_has_not_okay_status_code_then_response_is_not_successful)
{
    std::stringstream ss;
    ss << forbidden_response;
    http::HttpResponse response(std::move(ss));

    EXPECT_FALSE(response.isSuccessful());
}

} // namespace