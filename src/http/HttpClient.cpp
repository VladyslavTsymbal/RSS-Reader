#include "http/HttpClient.hpp"
#include "http/HttpRequest.hpp"         // for HttpRequest, requestMethodTo...
#include "http/HttpResponse.hpp"        // for HttpResponse
#include "http/IHttpConnection.hpp"     // for IHttpConnection
#include "utils/Log.hpp"                // for LOG_ERROR
#include "utils/network/StatusCode.hpp" // for StatusCode

#include <format>      // for format
#include <sstream>     // for basic_stringstream, stringst...
#include <string>      // for string
#include <string_view> // for string_view

namespace http {

constexpr std::string_view LOG_TAG = "HttpClient";

static std::stringstream
httpRequestToBytes(const HttpRequest& request)
{
    const auto request_method = requestMethodToString(request.getRequestMethod());
    if (!request_method)
    {
        return {};
    }

    std::string request_str = std::format(
            "{} {} HTTP/1.1\x0D\x0AHost: {}\x0D\x0A\x43onnection: Close\x0D\x0A\x0D\x0A",
            *request_method,
            request.getUrl(),
            request.getHost());

    return std::stringstream(request_str);
}

std::optional<HttpResponse>
HttpClient::sendRequest(const IHttpConnection& connection, const HttpRequest& request)
{
    std::stringstream request_bytes = httpRequestToBytes(request);
    utils::network::StatusCode status = connection.sendBytes(request_bytes);

    if (status == utils::network::StatusCode::FAIL)
    {
        LOG_ERROR(LOG_TAG, "Failed to sendBytes.");
        return std::nullopt;
    }

    return HttpResponse(connection.receiveBytes());
}

} // namespace http