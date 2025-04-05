#pragma once

#include <sstream>
#include <unordered_map>
#include <vector>

namespace http {

using HttpHeaders = std::unordered_map<std::string, std::string>;

class HttpResponse
{
public:
    HttpResponse(std::stringstream response);

    const std::string&
    getBody() const;

    bool
    isSuccessful() const;

    std::optional<std::string>
    getHeader(const std::string& key) const;

    int
    getStatusCode() const;

    const std::string&
    getDescription() const;

private:
    void
    parseResponse(std::vector<std::string>& vec);

    void
    parseStatusLine(std::vector<std::string>& vec);

    void
    parseHeaders(std::vector<std::string>& vec);

    void
    parseBody(std::vector<std::string>& vec);

private:
    HttpHeaders m_headers;
    std::string m_body;
    std::string m_description;
    std::string m_http_version;
    int m_status_code{-1};
};

} // namespace http