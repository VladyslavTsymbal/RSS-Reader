#include "http/HttpConnectionHandler.hpp"
#include "http/HttpConnectionState.hpp"
#include "http/HttpHelpers.hpp"
#include "http/Constants.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpRequestMethod.hpp"

#include "network/StatusCode.hpp"
#include "network/Types.hpp"
#include "network/TcpConnection.hpp"
#include "network/NetworkHelpers.hpp"

#include <expected>
#include <cassert>
#include <fstream>
#include <sstream>

namespace {

constexpr size_t BUFFER_SIZE = 4096zu;

using network::StatusCode;
using network::BytesView;
using network::Bytes;
using network::TcpConnection;

} // namespace

namespace http {

HttpConnectionHandler::HttpConnectionHandler(std::unique_ptr<TcpConnection> connection)
    : m_connection(std::move(connection))
    , m_buffers(std::make_unique<HttpConnectionHandler::ConnectionBuffers>())
{
    assert(m_connection);
    assert(m_buffers);
}

HttpConnectionHandler::HttpConnectionHandler(HttpConnectionHandler&& other) noexcept
    : m_connection(std::move(other.m_connection))
    , m_buffers(std::move(other.m_buffers))
{
    assert(m_connection);
    assert(m_buffers);
}

HttpConnectionHandler&
HttpConnectionHandler::operator=(HttpConnectionHandler&& other) noexcept
{
    if (&other != this)
    {
        m_connection = std::move(other.m_connection);
        m_buffers = std::move(other.m_buffers);
        assert(m_connection);
        assert(m_buffers);
    }

    return *this;
}

bool
HttpConnectionHandler::checkHeadersReceived()
{
    const auto end_of_headers_pos = findEndOfHeaders(
            std::string_view(
                    reinterpret_cast<const char*>(m_buffers->in.data()), m_buffers->in.size()));
    // TODO: Separate parsing logic from checking
    if (end_of_headers_pos != std::string::npos)
    {
        m_are_headers_received = true;
        // Save the size of headers in bytes
        m_buffers->in_header_size = end_of_headers_pos + END_OF_HEADERS_SEQ.size();

        std::string_view headers_sv(
                reinterpret_cast<const char*>(m_buffers->in.data()), end_of_headers_pos);
        const auto content_length = getContentLength(headers_sv);
        if (content_length)
        {
            m_buffers->in_payload_size = *content_length;
        }
    }

    return m_are_headers_received;
}

// If data in the buffer is equals or more then sum of headers + payload, then all data is
// received.
bool
HttpConnectionHandler::checkPayloadReceived() const
{
    return m_buffers->in.size() >= (m_buffers->in_header_size + m_buffers->in_payload_size);
}

network::StatusCode
HttpConnectionHandler::readAvailable()
{
    StatusCode status;

    while (true)
    {
        const auto request_data = m_connection->receiveBytes(BUFFER_SIZE);
        if (!request_data)
        {
            status = request_data.error();
            break;
        }

        network::copyBytes(*request_data, m_buffers->in);
    }

    if (status == StatusCode::READ_ERROR)
    {
        // Unrecoverable error.
        return status;
    }

    if (!m_are_headers_received)
    {
        if (checkHeadersReceived())
        {
            if (m_is_payload_present && !checkPayloadReceived())
            {
                m_state = HttpConnectionState::RECEIVING_PAYLOAD;
            }
            else
            {
                m_state = HttpConnectionState::PROCESSING;
            }
        }
    }
    else if (m_is_payload_present && !m_is_payload_received)
    {
        m_is_payload_received = checkPayloadReceived();
        if (m_is_payload_received)
        {
            m_state = HttpConnectionState::PROCESSING;
        }
    }

    return StatusCode::OK;
}

network::StatusCode
HttpConnectionHandler::writeAvailable()
{
    StatusCode status;

    while (true)
    {
        const auto bytes_left = m_buffers->out_size - m_buffers->out_offset;
        const auto bytes_to_sent = std::min(bytes_left, BUFFER_SIZE);

        BytesView bytes(std::begin(m_buffers->out) + m_buffers->out_offset, bytes_to_sent);
        const auto bytes_sent = m_connection->sendBytes(bytes);
        if (!bytes_sent)
        {
            status = bytes_sent.error();
            break;
        }

        m_buffers->out_offset += *bytes_sent;
        if (writeFinished())
        {
            // FIXME: Remove data only from begin..offset. Now clear the vector
            // for the sake of simplicity
            m_buffers->out.clear();
            m_buffers->out_size = 0;
            m_buffers->out_offset = 0;

            m_state = HttpConnectionState::RECEIVING_HEADERS;

            return StatusCode::OK;
        }
    }

    if (status == StatusCode::WOULD_BLOCK)
    {
        m_state = HttpConnectionState::WAIT_TO_WRITE;
    }
    else if (status == StatusCode::WRITE_ERROR)
    {
        return status;
    }

    return StatusCode::OK;
}

bool
HttpConnectionHandler::writeFinished() const
{
    return m_buffers->out_offset == m_buffers->out_size;
}

void
HttpConnectionHandler::processData()
{
    // FIXME: This is just for testing.
    const auto request =
            HttpRequestBuilder().buildFromString(network::bytesToString(m_buffers->in));
    if (!request)
    {
        return;
    }

    const auto response = createResponse(*request);
    BytesView bytes = network::toBytesView(response);
    network::copyBytes(bytes, m_buffers->out);
    // TODO: Easy to forget to update the buffer out size
    m_buffers->out_size = response.size();

    m_buffers->in.clear();
    m_buffers->in_header_size = 0;
    m_buffers->in_payload_size = 0;

    m_state = HttpConnectionState::WRITING;
}

HttpConnectionState
HttpConnectionHandler::getState() const
{
    return m_state;
}

std::string
HttpConnectionHandler::createResponse(const HttpRequest& request) const
{
    constexpr std::string_view HTMX_RESPONSE =
            R"(
                <html>
                <head><script src="https://unpkg.com/htmx.org"></script></head>
                <body>
                    <button hx-get="/feed.xml" hx-target="#result">Get Feed</button>
                    <div id="result"></div>
                </body>
                </html>
            )";

    // TODO: Remove this hardcode and implement HttpResponseBuilder with toString method
    // This method should return HttpResponse or std::optional<HttpResponse>
    std::string response_content;
    if (request.getRequestMethod() == HttpRequestMethod::GET)
    {
        if (request.getRequestTarget() == "/feed.xml")
        {
            std::ifstream ifile("feed.xml");
            if (ifile.is_open())
            {
                std::stringstream ss;
                ss << ifile.rdbuf();
                response_content = ss.str();
            }
            else
            {
                // LOG_ERROR(LOG_TAG, "Couldn't open \"feed.xml\"");
            }
        }
        else
        {
            response_content = HTMX_RESPONSE;
        }
    }

    std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: " +
            std::to_string(response_content.size()) +
            "\r\n"
            "Connection: close\r\n"
            "\r\n" +
            response_content;

    return response;
}

} // namespace http