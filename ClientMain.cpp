#include "http/HttpClient.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"
#include "http/HttpConnectionFactory.hpp"
#include "http/HttpRequestMethod.hpp"

#include "network/NetworkUtils.hpp"
#include "network/NetworkHelpers.hpp"
#include "network/SysCallsWrapper.hpp"
#include "utils/log/Log.hpp"

namespace {

constexpr std::string_view LOG_TAG = "ClientMain";

}

int
main()
{
    using namespace http;

    auto syscall_wrappers = std::make_shared<network::SysCallsWrapper>();
    auto network_utils = std::make_shared<network::NetworkUtils>(std::move(syscall_wrappers));
    auto connection_factory = std::make_shared<HttpConnectionFactory>(std::move(network_utils));

    auto request = HttpRequestBuilder()
                           .setHost("127.0.0.1")
                           .setRequestType(HttpRequestMethod::GET)
                           .setRequestTarget("/feed.xml")
                           .setConnectionType(ConnectionType::CLOSE)
                           .build();
    if (!request)
    {
        LOG_FATAL(LOG_TAG, "Failed to build http request!");
        return 1;
    }

    HttpClient http_client(std::move(connection_factory));
    const auto response = http_client.sendRequest(*request);
    if (!response)
    {
        LOG_FATAL(LOG_TAG, "Failed to get response from server!");
        return 1;
    }

    auto body = response->getBody();
    if (body)
    {
        LOG_INFO(LOG_TAG, "Response data: \n{}", network::toStringView(*body));
    }
    else
    {
        LOG_WARN(LOG_TAG, "Body is not present in the response");
    }

    return 0;
}