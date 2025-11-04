#include "http/HttpClient.hpp"
#include "http/HttpRequest.hpp"         // for HttpRequest, requestMethodTo...
#include "http/HttpResponse.hpp"        // for HttpResponse
#include "http/IHttpConnection.hpp"     // for IHttpConnection
#include "utils/Log.hpp"                // for LOG_ERROR
#include "utils/network/StatusCode.hpp" // for StatusCode

#include <format>      // for format
#include <string>      // for string
#include <string_view> // for string_view

namespace {
constexpr std::string_view LOG_TAG = "HttpClient";
}

namespace http {

static std::optional<std::string>
prepareHttpRequest(const HttpRequest& request)
{
    const auto request_method = requestMethodToString(request.getRequestMethod());
    if (!request_method)
    {
        return std::nullopt;
    }

    return std::format(
            "{} {} HTTP/1.1\x0D\x0AHost: {}\x0D\x0A\x43onnection: Close\x0D\x0A\x0D\x0A",
            *request_method,
            request.getUrl(),
            request.getHost());
}

std::optional<HttpResponse>
HttpClient::sendRequest(const IHttpConnection& connection, const HttpRequest& request)
{
    LOG_INFO(LOG_TAG, "Preparing http request.");

    auto request_string = prepareHttpRequest(request);
    if (!request_string)
    {
        LOG_ERROR(LOG_TAG, "Failed to prepareHttpRequest.");
        return std::nullopt;
    }

    LOG_INFO(LOG_TAG, "Sending the http request.");
    LOG_DEBUG(LOG_TAG, "Request content:\n{}\n", *request_string);

    auto request_as_bytes = std::as_bytes(std::span(*request_string));
    utils::network::StatusCode status = connection.sendBytes(request_as_bytes);

    if (status == utils::network::StatusCode::FAIL)
    {
        LOG_ERROR(LOG_TAG, "Failed to sendBytes.");
        return std::nullopt;
    }

    LOG_INFO(LOG_TAG, "Receiving response from the server.")
    auto bytes = connection.receiveBytes();
    if (!bytes)
    {
        LOG_ERROR(LOG_TAG, "Failed to receiveBytes.");
        return std::nullopt;
    }

    LOG_INFO(LOG_TAG, "Creating http response.")
    std::string response_string(reinterpret_cast<const char*>(bytes->data()), bytes->size());
    if (response_string.empty())
    {
        LOG_WARN(LOG_TAG, "Response from server is empty.")
        return std::nullopt;
    }

    return HttpResponse(std::move(response_string));
}

} // namespace http