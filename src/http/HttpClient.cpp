#include "http/HttpClient.hpp"
#include "http/HttpConnectionFactory.hpp"
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
using utils::network::StatusCode;

} // namespace

namespace http {

HttpClient::HttpClient(std::shared_ptr<HttpConnectionFactory> factory)
    : m_factory(std::move(factory))
{
    assert(m_factory);
}

static std::optional<std::string>
prepareHttpRequest(const HttpRequest& request)
{
    const auto request_method = requestMethodToString(request.getRequestMethod());
    if (!request_method)
    {
        return std::nullopt;
    }

    return std::format(
            "{1} {2} HTTP/1.1{0}"
            "Host: {3}{0}"
            "Content-Length: 0{0}"
            "Connection: Close{0}"
            "{0}",
            CRLF,
            *request_method,
            request.getUrl(),
            request.getHost());
}

std::optional<HttpResponse>
HttpClient::sendRequest(const HttpRequest& request)
{
    using utils::network::statusCodeToError;
    LOG_INFO(LOG_TAG, "Preparing http request.");

    auto request_string = prepareHttpRequest(request);
    if (!request_string)
    {
        LOG_ERROR(LOG_TAG, "Failed to prepareHttpRequest.");
        return std::nullopt;
    }

    LOG_INFO(LOG_TAG, "Sending the http request.");
    LOG_DEBUG(LOG_TAG, "Request content:\n{}\n", *request_string);

    auto connection = m_factory->createConnection("127.0.0.1", 8080);
    const StatusCode status = connection->sendData(*request_string);
    if (status != StatusCode::OK)
    {
        LOG_ERROR(LOG_TAG, statusCodeToError(status));
        return std::nullopt;
    }

    LOG_INFO(LOG_TAG, "Receiving response from the server.")
    auto response = connection->receiveData();
    if (!response)
    {
        const StatusCode error = response.error();
        LOG_ERROR(LOG_TAG, statusCodeToError(error));
        return std::nullopt;
    }
    connection->closeConnection();

    LOG_INFO(LOG_TAG, "Creating http response.")
    if (response->empty())
    {
        LOG_WARN(LOG_TAG, "Response from server is empty.")
        return std::nullopt;
    }

    return HttpResponse(std::move(*response));
}

} // namespace http