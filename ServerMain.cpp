#include "http/HttpConnectionFactory.hpp"
#include "http/HttpServer.hpp"
#include "network/NetworkUtils.hpp"
#include "network/SysCallsWrapper.hpp"
#include "utils/log/Log.hpp"

namespace {
constexpr std::string_view LOG_TAG = "ServerMain";
}

int
main()
{
    using namespace http;

    auto syscall_wrappers = std::make_shared<network::SysCallsWrapper>();
    auto network_utils = std::make_shared<network::NetworkUtils>(std::move(syscall_wrappers));
    auto connection_factory = std::make_shared<http::HttpConnectionFactory>(network_utils);

    HttpServer http_server(
            "0.0.0.0", 8080, std::move(network_utils), std::move(connection_factory));
    const bool is_server_inited = http_server.init();
    if (!is_server_inited)
    {
        LOG_ERROR(LOG_TAG, "Server's initialization failed. Stopping.");
        return 1;
    }

    http_server.run();

    return 0;
}