#pragma once

#include "spdlog/spdlog.h"

#include <string_view>

namespace utils {

class LoggerImpl
{
public:
    static LoggerImpl&
    getInstance()
    {
        static LoggerImpl instance;
        return instance;
    }

    template <typename... Args>
    void
    logFatal(std::string_view tag, std::string_view fmt, Args&&... args);

    template <typename... Args>
    void
    logError(std::string_view tag, std::string_view fmt, Args&&... args);

    template <typename... Args>
    void
    logWarn(std::string_view tag, std::string_view fmt, Args&&... args);

    template <typename... Args>
    void
    logInfo(std::string_view tag, std::string_view fmt, Args&&... args);

    template <typename... Args>
    void
    logDebug(std::string_view tag, std::string_view fmt, Args&&... args);

    template <typename... Args>
    void
    logTrace(std::string_view tag, std::string_view fmt, Args&&... args);

private:
    static std::string
    createFormatString(std::string_view tag, std::string_view fmt);

    LoggerImpl()
    {
        // Change log pattern
        spdlog::set_pattern("[%d-%b %T.%e] [%l] %v");
        // Set `DEBUG` level for debug builds
#ifndef NDEBUG
        spdlog::set_level(spdlog::level::debug);
#endif
    }
};

inline std::string
LoggerImpl::createFormatString(std::string_view tag, std::string_view fmt)
{
    return "[" + std::string(tag) + "] " + std::string(fmt);
}

template <typename... Args>
void
LoggerImpl::logFatal(std::string_view tag, std::string_view fmt, Args&&... args)
{
    // Forced to use `fmt::runtime` as `createFormatString` can't be `constexpr` function.
    // I didn't found a straight-forward way to display tag without something like
    // `createFormatString`.
    spdlog::critical(
            fmt::runtime(LoggerImpl::createFormatString(tag, fmt)), std::forward<Args>(args)...);
}

template <typename... Args>
void
LoggerImpl::logError(std::string_view tag, std::string_view fmt, Args&&... args)
{
    spdlog::error(
            fmt::runtime(LoggerImpl::createFormatString(tag, fmt)), std::forward<Args>(args)...);
}

template <typename... Args>
void
LoggerImpl::logWarn(std::string_view tag, std::string_view fmt, Args&&... args)
{
    spdlog::warn(
            fmt::runtime(LoggerImpl::createFormatString(tag, fmt)), std::forward<Args>(args)...);
}

template <typename... Args>
void
LoggerImpl::logInfo(std::string_view tag, std::string_view fmt, Args&&... args)
{
    spdlog::info(
            fmt::runtime(LoggerImpl::createFormatString(tag, fmt)), std::forward<Args>(args)...);
}

template <typename... Args>
void
LoggerImpl::logDebug(std::string_view tag, std::string_view fmt, Args&&... args)
{
    spdlog::debug(
            fmt::runtime(LoggerImpl::createFormatString(tag, fmt)), std::forward<Args>(args)...);
}

template <typename... Args>
void
LoggerImpl::logTrace(std::string_view tag, std::string_view fmt, Args&&... args)
{
    spdlog::trace(
            fmt::runtime(LoggerImpl::createFormatString(tag, fmt)), std::forward<Args>(args)...);
}

} // namespace utils