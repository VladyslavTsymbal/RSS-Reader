#include "http/HttpClient.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpConnectionFactory.hpp"
#include "http/IHttpConnection.hpp"
#include "utils/network/NetworkUtils.hpp"
#include "utils/network/SysCallsWrapper.hpp"
#include "utils/Log.hpp"

constexpr std::string_view LOG_TAG = "main";

int
main()
{
    using namespace http;

    auto syscall_wrappers = std::make_shared<utils::network::SysCallsWrapper>();
    auto network_utils = std::make_shared<utils::network::NetworkUtils>(syscall_wrappers);

    HttpConnectionFactory connection_factory(network_utils);
    auto connection = connection_factory.createConnection("127.0.0.1", 8080);

    if (!connection)
    {
        LOG_ERROR(LOG_TAG, "Failed to create connection.");
        return 1;
    }

    HttpRequest request = HttpRequestBuilder()
                                  .setHost("127.0.0.1")
                                  .setRequestType(HttpRequest::HttpRequestMethod::GET)
                                  .setRequestUrl("/feed.xml")
                                  .build();

    HttpClient http_client;
    const auto response = http_client.sendRequest(*connection, request);
    if (!response)
    {
        LOG_ERROR(LOG_TAG, "Failed to getResponse!");
        return 1;
    }

    LOG_INFO(LOG_TAG, "Response data: {}", response->getData());

    return 0;
}