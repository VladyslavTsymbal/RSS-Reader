#pragma once

#include <memory>
#include <functional>
#include <netdb.h>
#include <span>
#include <vector>

namespace utils::network {

using AddrInfoPtr = std::unique_ptr<addrinfo, std::function<void(addrinfo*)>>;
using Socket = std::unique_ptr<int, std::function<int(int*)>>;
using Bytes = std::vector<std::byte>;
using BytesView = std::span<const std::byte>;

} // namespace utils::network