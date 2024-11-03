#pragma once

#include <sstream>

namespace http {

class HttpResponse
{
public:
    HttpResponse(std::stringstream response)
        : m_response(std::move(response))
    {
    }

    std::string
    getData() const
    {
        return m_response.str();
    }

private:
    std::stringstream m_response;
};

} // namespace http