#include "http/HttpClient.hpp"
#include "http/Constants.hpp"
#include "http/HttpConnectionFactory.hpp"
#include "http/HttpRequest.hpp"  // for HttpRequest, requestMethodTo...
#include "http/HttpResponse.hpp" // for HttpResponse
#include "http/HttpHelpers.hpp"

#include "network/NetworkHelpers.hpp"
#include "network/TcpConnection.hpp"
#include "network/StatusCode.hpp" // for StatusCode
#include "network/Types.hpp"
#include "utils/log/Log.hpp" // for LOG_ERROR

#include <optional>
#include <string>      // for string
#include <string_view> // for string_view

namespace {

constexpr std::string_view LOG_TAG = "HttpClient";
constexpr size_t BUFFER_SIZE = 4096uz;

using network::StatusCode;
using network::BytesView;
using network::Bytes;

size_t
checkHeadersReceived(BytesView buffer)
{
    auto end_of_headers_pos = http::findEndOfHeaders(
            std::string_view(reinterpret_cast<const char*>(buffer.data()), buffer.size()));

    if (end_of_headers_pos != std::string::npos)
    {
        end_of_headers_pos += http::END_OF_HEADERS_SEQ.size();
    }

    return end_of_headers_pos;
}

StatusCode
sendRequestSync(
        const std::unique_ptr<network::TcpConnection>& connection, std::string_view request_string)
{
    StatusCode status{StatusCode::OK};
    BytesView request_bytes = network::toBytesView(request_string);
    size_t offset = 0zu;

    while (true)
    {
        const auto bytes_left = request_bytes.size() - offset;
        const auto bytes_to_sent = std::min(bytes_left, BUFFER_SIZE);

        if (bytes_left == 0)
        {
            // Finished writting
            break;
        }

        BytesView chunk(std::begin(request_bytes) + offset, bytes_to_sent);
        const auto bytes_sent = connection->sendBytes(chunk);
        if (!bytes_sent)
        {
            status = bytes_sent.error();
            break;
        }

        offset += *bytes_sent;
    }

    return status;
}

std::expected<Bytes, StatusCode>
receiveResponseSync(const std::unique_ptr<network::TcpConnection>& connection)
{
    Bytes response_bytes;
    auto end_of_headers_pos = std::string::npos;
    size_t content_length = 0;
    size_t bytes_to_read = 0;
    bool headers_recieved = false;

    do
    {
        const auto request_data = connection->receiveBytes(BUFFER_SIZE);
        if (!request_data)
        {
            return std::unexpected(request_data.error());
        }

        network::copyBytes(response_bytes, *request_data);
        if (!headers_recieved)
        {
            const auto eofh_pos = checkHeadersReceived(response_bytes);
            if (eofh_pos == std::string::npos)
            {
                // Headers are not received yet, continue to receive data.
                continue;
            }

            end_of_headers_pos = eofh_pos;
            headers_recieved = true;

            std::string_view headers_sv(
                    reinterpret_cast<const char*>(response_bytes.data()), end_of_headers_pos);

            const auto content_len = http::getContentLength(headers_sv);
            if (!content_len)
            {
                // There is no data about size of the body, assuming that body is missing.
                // We already received headers, body is missing (assumption) -> all data received.
                break;
            }
            content_length = *content_len;
        }

        bytes_to_read = end_of_headers_pos + content_length - response_bytes.size();
        if (bytes_to_read == 0)
        {
            // All data received.
            break;
        }
    } while (true);

    return response_bytes;
}

} // namespace

namespace http {

HttpClient::HttpClient(std::shared_ptr<HttpConnectionFactory> factory)
    : m_factory(std::move(factory))
{
    assert(m_factory);
}

std::optional<HttpResponse>
HttpClient::sendRequest(const HttpRequest& request)
{
    using network::statusCodeToError;
    LOG_INFO(LOG_TAG, "Preparing http request.");

    auto request_string = request.toString();
    if (request_string.empty())
    {
        LOG_ERROR(LOG_TAG, "Failed to convert request to string.");
        return std::nullopt;
    }

    auto connection = m_factory->createTcpConnection("127.0.0.1", 8080);
    if (!connection)
    {
        LOG_ERROR(LOG_TAG, "Failed to createConnection.");
        return std::nullopt;
    }

    LOG_INFO(LOG_TAG, "Sending the http request.");
    LOG_DEBUG(LOG_TAG, "Request content:\n{}\n", request_string);

    StatusCode status = sendRequestSync(connection, request_string);
    if (status != StatusCode::OK)
    {
        LOG_ERROR(LOG_TAG, statusCodeToError(status));
        return std::nullopt;
    }

    // Receiving the response
    LOG_INFO(LOG_TAG, "Receiving response from the server.")
    auto response_bytes = receiveResponseSync(connection);
    if (!response_bytes)
    {
        LOG_ERROR(LOG_TAG, "Failed to receive response from the server.");
        return std::nullopt;
    }

    if (response_bytes->empty())
    {
        LOG_ERROR(LOG_TAG, "Response from server is empty.")
        return std::nullopt;
    }

    LOG_INFO(LOG_TAG, "Creating http response.")
    HttpResponse http_response(std::move(*response_bytes));
    if (!http_response.isValid())
    {
        LOG_ERROR(LOG_TAG, "Failed to parse response from server.")
        return std::nullopt;
    }

    connection->close();
    return http_response;
}

} // namespace http