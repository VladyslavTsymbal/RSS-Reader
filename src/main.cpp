#include "http/HttpClient.hpp"
#include "http/HttpRequest.hpp"
#include "utils/Log.hpp"

constexpr std::string_view LOG_TAG = "main";

int
main()
{
    using namespace http;

    HttpClient http_client;
    auto connection = http_client.createConnection("127.0.0.1", 8080);
    if (!connection)
    {
        LOG_ERROR(LOG_TAG, "Failed to create connection.");
        return 1;
    }

    HttpRequest request = HttpRequestBuilder()
                                  .setRequestType(HttpRequest::HttpRequestMethod::GET)
                                  .setRequestUrl("/feed.xml")
                                  .build();

    const auto response = http_client.getResponse(*connection, request);
    if (!response)
    {
        LOG_ERROR(LOG_TAG, "Failed to getResponse!");
        return 1;
    }

    LOG_INFO(LOG_TAG, "Response data: {}", response->getData());

    http_client.closeConnection(*connection);

    return 0;
}