#include "http/HttpResponse.hpp"

#include <algorithm>
#include <boost/algorithm/string_regex.hpp>
#include <iterator>
#include <optional>

namespace http {

auto
findBeginningOfBody(const std::vector<std::string>& vec)
{
    return std::find_if(std::begin(vec), std::end(vec), [](const auto& str) {
        if (str.empty())
        {
            return true;
        }

        return false;
    });
}

std::vector<std::string>
splitResponseByControlSequence(std::string response)
{
    std::vector<std::string> splitted_response;
    boost::algorithm::split_regex(splitted_response, response, boost::regex("\r\n"));

    return splitted_response;
}

bool
HttpResponse::isSuccessful() const
{
    return (m_status_code >= 200 && m_status_code <= 208) ? true : false;
}

void
HttpResponse::parseResponse(std::vector<std::string>& vec)
{
    // Move from the end to the beggining in order to avoid
    // extra swaps inside of the vector after pop.
    parseBody(vec);
    parseHeaders(vec);
    parseStatusLine(vec);
}

void
HttpResponse::parseStatusLine(std::vector<std::string>& vec)
{
    if (!vec.empty())
    {
        std::string status_line = vec.back();
        vec.pop_back();

        boost::regex status_line_regex("HTTP\\/([0-9]\\.[0-9])\\s([1-5][0-9]{2})\\s(.*)");
        // This will hold the results
        boost::match_results<std::string::const_iterator> matches;

        std::string::const_iterator start = status_line.begin();
        std::string::const_iterator end = status_line.end();

        const bool found = boost::regex_search(start, end, matches, status_line_regex);
        if (found)
        {
            m_http_version = matches[1];
            m_status_code = std::stoi(matches[2], nullptr);
            m_description = matches[3];
        }
    }
}

void
HttpResponse::parseHeaders(std::vector<std::string>& vec)
{
    while (!vec.empty())
    {
        std::vector<std::string> splitted_header;
        boost::algorithm::split_regex(splitted_header, vec.back(), boost::regex(": "));

        if (splitted_header.size() != 2)
        {
            // We reached status line, stop there
            break;
        }

        const auto key = splitted_header[0];
        const auto value = splitted_header[1];
        m_headers[key] = value;
        vec.pop_back();
    }
}

void
HttpResponse::parseBody(std::vector<std::string>& vec)
{
    // IDK, but `boost::split_regex` adds new empty line in case
    // response does not have a body. So, I dont need to check if
    // body present or not and can blindly remove the "\r\n" separator.
    m_body = vec.back();
    vec.pop_back();
    // Drop "\r\n"
    vec.pop_back();
}

HttpResponse::HttpResponse(std::string response)
{
    if (!response.empty())
    {
        auto response_splitted_in_strings = splitResponseByControlSequence(std::move(response));
        parseResponse(response_splitted_in_strings);
    }
}

const std::string&
HttpResponse::getBody() const
{
    return m_body;
}

std::optional<std::string>
HttpResponse::getHeader(const std::string& key) const
{
    if (m_headers.find(key) != std::end(m_headers))
    {
        return m_headers.at(key);
    }

    return std::nullopt;
}

int
HttpResponse::getStatusCode() const
{
    return m_status_code;
}

const std::string&
HttpResponse::getDescription() const
{
    return m_description;
}

} // namespace http