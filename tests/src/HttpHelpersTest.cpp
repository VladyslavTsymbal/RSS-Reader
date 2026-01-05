#include "http/HttpHelpers.hpp"
#include "http/Types.hpp"

#include <gmock/gmock.h>
#include <optional>

namespace {

static constexpr std::string_view valid_response =
        "Date: Tue, 02 Apr 2024 12:00:00 GMT\r\n"
        "Server: ExampleServer/1.0\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "Content-Length: 88\r\n"
        "Connection: keep-alive\r\n"
        "\r\n";

static constexpr std::string_view invalid_response =
        "Content-Type:text/html\r\n"
        "Date: Tue, 02 Apr 2024 12:00:00 GMT" // Missing delim at the end of line
        "Server ExampleServer/1.0\r\n"        // Missing semicolon
        "Connection: keep-alive\r\n";         // Missing end of header delim

TEST(HttpHelpersTest, when_empty_string_passed_then_npos_returned)
{
    const auto pos = http::findEndOfHeaders("");
    ASSERT_EQ(pos, std::string::npos);
}

TEST(HttpHelpersTest, when_eoh_present_then_valid_position_returned)
{
    const auto pos = http::findEndOfHeaders(valid_response);
    ASSERT_NE(pos, std::string::npos);
}

TEST(HttpHelpersTest, when_eoh_not_present_then_npos_returned)
{
    const auto pos = http::findEndOfHeaders(invalid_response);
    ASSERT_EQ(pos, std::string::npos);
}

TEST(HttpHelpersTest, when_content_length_present_then_valid_position_returned)
{
    const auto pos = http::getContentLength(valid_response);
    ASSERT_TRUE(pos);
    EXPECT_NE(*pos, std::string::npos);
}

TEST(HttpHelpersTest, when_content_length_not_present_then_nullopt_returned)
{
    const auto pos = http::getContentLength(invalid_response);
    ASSERT_FALSE(pos);
}

TEST(HttpHelpersTest, when_string_is_empty_then_headers_are_empty)
{
    const auto headers = http::parseHeaders("");
    EXPECT_TRUE(headers.empty());
}

TEST(HttpHelpersTest, when_headers_are_valid_then_all_headers_are_present)
{
    const auto headers = http::parseHeaders(valid_response);
    ASSERT_FALSE(headers.empty());
    EXPECT_EQ(headers.at("Date"), "Tue, 02 Apr 2024 12:00:00 GMT");
    EXPECT_EQ(headers.at("Server"), "ExampleServer/1.0");
    EXPECT_EQ(headers.at("Content-Type"), "text/html; charset=UTF-8");
    EXPECT_EQ(headers.at("Content-Length"), "88");
    EXPECT_EQ(headers.at("Connection"), "keep-alive");
}

TEST(HttpHelpersTest, when_get_headers_are_present_then_correct_values_returned)
{
    http::HttpHeaders headers;
    headers["Date"] = "Tue, 02 Apr 2024 12:00:00 GMT";
    headers["Server"] = "ExampleServer/1.0";
    headers["Content-Type"] = "text/html; charset=UTF-8";
    headers["Content-Length"] = "88";
    headers["Connection"] = "keep-alive";

    EXPECT_EQ(http::getValueFromHeader(headers, "Date"), "Tue, 02 Apr 2024 12:00:00 GMT");
    EXPECT_EQ(http::getValueFromHeader(headers, "Server"), "ExampleServer/1.0");
    EXPECT_EQ(http::getValueFromHeader(headers, "Content-Type"), "text/html; charset=UTF-8");
    EXPECT_EQ(http::getValueFromHeader(headers, "Content-Length"), "88");
    EXPECT_EQ(http::getValueFromHeader(headers, "Connection"), "keep-alive");
}

TEST(HttpHelpersTest, when_headers_not_present_then_nullopt_returned)
{
    http::HttpHeaders headers;
    headers["Date"] = "Tue, 02 Apr 2024 12:00:00 GMT";
    headers["Server"] = "ExampleServer/1.0";

    EXPECT_EQ(http::getValueFromHeader(headers, "Connection"), std::nullopt);
    EXPECT_EQ(http::getValueFromHeader(headers, "Content-Length"), std::nullopt);
}

TEST(HttpHelpersTest, when_empty_headers_are_passed_then_string_is_empty)
{
    http::HttpHeaders headers;
    const auto headers_str = http::headersToString(headers);
    EXPECT_EQ(headers_str, "");
}

} // namespace
