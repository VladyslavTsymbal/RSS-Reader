#pragma once

#include "http/HttpClient.hpp"
#include "http/HttpConnection.hpp"
#include "utils/INetworkUtils.hpp"

#include <gmock/gmock.h>

namespace {

using http::HttpConnection;

struct MockHttpClient : public http::HttpClient
{
    MockHttpClient(std::shared_ptr<utils::network::INetworkUtils> network_utils)
        : HttpClient(std::move(network_utils))
    {
    }

    MOCK_METHOD(
            utils::network::StatusCode,
            sendRequestImpl,
            (const int socket_fd, const std::string&),
            (override));

    MOCK_METHOD(std::stringstream, getResponseImpl, (const int), (override));

    utils::network::INetworkUtils*
    getNetworkUtils()
    {
        return m_network_utils.get();
    }
};

} // namespace