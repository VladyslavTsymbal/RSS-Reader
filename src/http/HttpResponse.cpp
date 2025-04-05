#include "http/HttpResponse.hpp"

#include <algorithm>
#include <boost/algorithm/string_regex.hpp>
#include <iterator>

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
splitResponseByControlSequence(std::stringstream& response)
{
    std::string data = response.str();

    std::vector<std::string> splitted_response;
    boost::algorithm::split_regex(splitted_response, data, boost::regex("\r\n"));

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
    m_body = vec.back();
    vec.pop_back();

    if (!m_body.empty())
    {
        // If body is not empty then we also need to remove '\r\n',
        // which separates headers and body.
        vec.pop_back();
    }
}

HttpResponse::HttpResponse(std::stringstream response)
{
    auto response_splitted_in_strings = splitResponseByControlSequence(response);
    parseResponse(response_splitted_in_strings);
}

const std::string&
HttpResponse::getBody() const
{
    return m_body;
}

} // namespace http