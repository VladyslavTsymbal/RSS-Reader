#include "http/HttpClient.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpConnectionFactory.hpp"
#include "http/IHttpConnection.hpp"
#include "utils/network/NetworkUtils.hpp"
#include "utils/network/SysCallsWrapper.hpp"
#include "utils/Log.hpp"

namespace {
constexpr std::string_view LOG_TAG = "ClientMain";
}

int
main()
{
    using namespace http;

    auto syscall_wrappers = std::make_shared<utils::network::SysCallsWrapper>();
    auto network_utils =
            std::make_shared<utils::network::NetworkUtils>(std::move(syscall_wrappers));
    auto connection_factory = std::make_shared<HttpConnectionFactory>(std::move(network_utils));

    HttpRequest request = HttpRequestBuilder()
                                  .setHost("127.0.0.1")
                                  .setRequestType(HttpRequest::HttpRequestMethod::GET)
                                  .setRequestUrl("/tests/feed.xml")
                                  .build();

    HttpClient http_client(std::move(connection_factory));
    const auto response = http_client.sendRequest(request);
    if (!response)
    {
        LOG_ERROR(LOG_TAG, "Failed to get response from server!");
        return 1;
    }

    auto body = response->getBody();
    if (body)
    {
        LOG_INFO(LOG_TAG, "Response data: \n{}", *body);
    }
    else
    {
        LOG_WARN(LOG_TAG, "Body is not present in the response");
    }

    return 0;
}