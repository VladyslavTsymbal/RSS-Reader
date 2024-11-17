#include "http/HttpClient.hpp"
#include "http/HttpRequest.hpp"

#include <iostream>

int
main()
{
    using namespace http;

    HttpClient http_client;
    auto connection = http_client.createConnection("127.0.0.1", 8080);
    if (!connection)
    {
        std::cerr << "Failed to create connection.\n";
        return 1;
    }

    HttpRequest request = HttpRequestBuilder()
                                  .setRequestType(HttpRequest::HttpRequestMethod::GET)
                                  .setRequestUrl("/feed.xml")
                                  .build();

    const auto response = http_client.getResponse(*connection, request);
    if (!response)
    {
        std::cerr << "Failed to getResponse!\n";
        return 1;
    }

    std::cout << response->getData() << std::endl;

    connection->closeConnection();

    return 0;
}