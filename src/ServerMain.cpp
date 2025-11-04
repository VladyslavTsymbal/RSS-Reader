#include "http/HttpServer.hpp"
#include "utils/network/NetworkUtils.hpp"
#include "utils/network/SysCallsWrapper.hpp"
#include "utils/Log.hpp"

namespace {
constexpr std::string_view LOG_TAG = "ServerMain";
}

int
main()
{
    using namespace http;

    auto syscall_wrappers = std::make_shared<utils::network::SysCallsWrapper>();
    auto network_utils =
            std::make_shared<utils::network::NetworkUtils>(std::move(syscall_wrappers));

    HttpServer http_server("127.0.0.1", 8080, std::move(network_utils));
    const bool is_server_inited = http_server.init();
    if (!is_server_inited)
    {
        LOG_ERROR(LOG_TAG, "Server's initialization failed. Stopping.");
        return 1;
    }

    http_server.run();

    return 0;
}