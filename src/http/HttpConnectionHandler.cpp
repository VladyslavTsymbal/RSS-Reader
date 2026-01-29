#include "http/HttpConnectionHandler.hpp"
#include "http/HttpConnectionState.hpp"
#include "http/HttpHelpers.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpRequestMethod.hpp"

#include "network/PollEvents.hpp"
#include "network/StatusCode.hpp"
#include "network/Types.hpp"
#include "network/TcpConnection.hpp"
#include "network/NetworkHelpers.hpp"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <sstream>
#include <type_traits>

namespace {

constexpr size_t BUFFER_SIZE = 4096zu;

using network::StatusCode;
using network::BytesView;
using network::Bytes;
using network::TcpConnection;
using network::event::PollEvent;
using network::event::Read;
using network::event::Write;
using network::event::Close;

} // namespace

namespace http {

struct ConnectionBuffers
{
    network::Bytes in;
    network::Bytes out;

    size_t in_header_size{};
    size_t in_payload_size{};

    size_t out_offset{};
    size_t out_size{};
};

// If data in the buffer is equals or more then sum of headers + payload, then all data is
// received.
bool
checkPayloadReceived(const http::ConnectionBuffers& buffers)
{
    return buffers.in.size() >= (buffers.in_header_size + buffers.in_payload_size);
}

bool
writeFinished(const http::ConnectionBuffers& buffers)
{
    return buffers.out_offset == buffers.out_size;
}

std::string
createResponse(const HttpRequest& request)
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

struct HandlerStateVisitor
{
    HandlerStateVisitor(HttpConnectionHandler& handler)
        : connection(*handler.m_connection)
        , buffers(*handler.m_buffers)
        , handler_state(handler.m_state)
    {
    }

    template <typename State, typename Event>
    HandlerInterest
    operator()(State& state, Event&) const
    {
        using StateType = std::remove_cvref_t<State>;
        using EventType = std::remove_cvref_t<Event>;

        if constexpr (std::is_same_v<EventType, Close>)
        {
            return changeState(Closing{});
        }
        else if constexpr (std::is_same_v<StateType, Closing>)
        {
            return handleClosing(state);
        }
        else if constexpr (
                std::is_same_v<StateType, ReadyToRead> && std::is_same_v<EventType, Read>)
        {
            return changeState(Receiving{});
        }
        else if constexpr (std::is_same_v<StateType, Receiving> && std::is_same_v<EventType, Read>)
        {
            return handleReceiving(state);
        }
        else if constexpr (std::is_same_v<StateType, Processing>)
        {
            return handleProcessing();
        }
        else if constexpr (
                std::is_same_v<StateType, WaitingToWrite> && std::is_same_v<EventType, Write>)
        {
            return changeState(Writing{});
        }
        else if constexpr (std::is_same_v<StateType, Writing> && std::is_same_v<EventType, Write>)
        {
            return handleWriting();
        }

        return state.interest;
    }

private:
    HandlerInterest
    handleClosing(Closing& state) const
    {
        // We should drain all the data from the connection before closing it (if possible)
        StatusCode status;
        while (true)
        {
            const auto request_data = connection.tryReceiveBytes(BUFFER_SIZE);
            if (!request_data)
            {
                status = request_data.error();
                break;
            }

            network::copyBytes(buffers.in, *request_data);
        }

        if (status != StatusCode::WOULD_BLOCK)
        {
        }

        return state.interest;
    }

    HandlerInterest
    handleReceiving(Receiving& state) const
    {
        StatusCode status;
        while (true)
        {
            const auto request_data = connection.tryReceiveBytes(BUFFER_SIZE);
            if (!request_data)
            {
                status = request_data.error();
                break;
            }

            network::copyBytes(buffers.in, *request_data);
        }

        if (status == StatusCode::READ_ERROR)
        {
            // Unrecoverable error.
            return changeState(Closing{});
        }

        const auto headers_end_pos = getEndOfHeaders(buffers.in);
        if (headers_end_pos == std::string::npos)
        {
            return state.interest;
        }

        // Save end_ofh to the connection buffer struct
        buffers.in_header_size = headers_end_pos;

        std::string_view headers_sv(
                reinterpret_cast<const char*>(buffers.in.data()), headers_end_pos);
        const auto content_length = getContentLength(headers_sv);
        if (!content_length)
        {
            // No payload. Start to process the request.
            return changeState(Processing{});
        }

        // Save payload size
        buffers.in_payload_size = *content_length;
        if (!checkPayloadReceived(buffers))
        {
            return state.interest;
        }

        return changeState(Processing{});
    }

    HandlerInterest
    handleProcessing() const
    {
        // FIXME: This is just for testing.
        const auto request =
                HttpRequestBuilder().buildFromString(network::toStringView(buffers.in));
        if (!request)
        {
            return changeState(Closing{});
        }

        const auto response = createResponse(*request);
        BytesView bytes = network::toBytesView(response);
        network::copyBytes(buffers.out, bytes);
        // TODO: Easy to forget to update the buffer out size
        buffers.out_size = response.size();

        buffers.in.clear();
        buffers.in_header_size = 0;
        buffers.in_payload_size = 0;

        return changeState(Writing{});
    }

    HandlerInterest
    handleWriting() const
    {
        StatusCode status;
        while (true)
        {
            const auto bytes_left = buffers.out_size - buffers.out_offset;
            const auto bytes_to_sent = std::min(bytes_left, BUFFER_SIZE);

            BytesView bytes(std::begin(buffers.out) + buffers.out_offset, bytes_to_sent);
            const auto bytes_sent = connection.trySendBytes(bytes);
            if (!bytes_sent)
            {
                status = bytes_sent.error();
                break;
            }

            buffers.out_offset += *bytes_sent;
            if (writeFinished(buffers))
            {
                // FIXME: Remove data only from begin..offset. Now clear the vector
                // for the sake of simplicity
                buffers.out.clear();
                buffers.out_size = 0;
                buffers.out_offset = 0;

                return changeState(ReadyToRead{});
            }
        }

        if (status == StatusCode::WOULD_BLOCK)
        {
            return changeState(WaitingToWrite{});
        }
        if (status == StatusCode::WRITE_ERROR)
        {
            return changeState(Closing{});
        }

        return changeState(Writing{});
    }

    template <typename State>
    HandlerInterest
    changeState(State new_state) const
    {
        handler_state = new_state;
        return new_state.interest;
    }

    TcpConnection& connection;
    ConnectionBuffers& buffers;
    HandlerState& handler_state;
};

HttpConnectionHandler::~HttpConnectionHandler() = default;

HttpConnectionHandler::HttpConnectionHandler(std::unique_ptr<TcpConnection> connection)
    : m_connection(std::move(connection))
    , m_buffers(std::make_unique<ConnectionBuffers>())
    , m_state(ReadyToRead{})
{
    assert(m_connection);
    assert(m_buffers);
}

HttpConnectionHandler::HttpConnectionHandler(HttpConnectionHandler&& other) noexcept
    : m_connection(std::move(other.m_connection))
    , m_buffers(std::move(other.m_buffers))
    , m_state(ReadyToRead{})
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

HandlerInterest
HttpConnectionHandler::handleEvent(PollEvent event)
{
    return std::visit(HandlerStateVisitor(*this), m_state, event);
}

} // namespace http
