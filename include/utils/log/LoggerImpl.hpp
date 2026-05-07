#pragma once

#include <spdlog/spdlog.h>

#include <string>
#include <string_view>
#include <utility>

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

    template <typename... Args>
    static std::string
    createMessage(std::string_view tag, std::string_view fmt, Args&&... args);

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
std::string
LoggerImpl::createMessage(std::string_view tag, std::string_view fmt, Args&&... args)
{
    auto format_string = LoggerImpl::createFormatString(tag, fmt);
    if constexpr (sizeof...(Args) == 0)
    {
        return format_string;
    }
    else
    {
        return spdlog::fmt_lib::format(
                fmt::runtime(format_string), std::forward<Args>(args)...);
    }
}

template <typename... Args>
void
LoggerImpl::logFatal(std::string_view tag, std::string_view fmt, Args&&... args)
{
    spdlog::default_logger_raw()->log(
            spdlog::level::critical,
            spdlog::string_view_t(LoggerImpl::createMessage(tag, fmt, std::forward<Args>(args)...)));
}

template <typename... Args>
void
LoggerImpl::logError(std::string_view tag, std::string_view fmt, Args&&... args)
{
    spdlog::default_logger_raw()->log(
            spdlog::level::err,
            spdlog::string_view_t(LoggerImpl::createMessage(tag, fmt, std::forward<Args>(args)...)));
}

template <typename... Args>
void
LoggerImpl::logWarn(std::string_view tag, std::string_view fmt, Args&&... args)
{
    spdlog::default_logger_raw()->log(
            spdlog::level::warn,
            spdlog::string_view_t(LoggerImpl::createMessage(tag, fmt, std::forward<Args>(args)...)));
}

template <typename... Args>
void
LoggerImpl::logInfo(std::string_view tag, std::string_view fmt, Args&&... args)
{
    spdlog::default_logger_raw()->log(
            spdlog::level::info,
            spdlog::string_view_t(LoggerImpl::createMessage(tag, fmt, std::forward<Args>(args)...)));
}

template <typename... Args>
void
LoggerImpl::logDebug(std::string_view tag, std::string_view fmt, Args&&... args)
{
    spdlog::default_logger_raw()->log(
            spdlog::level::debug,
            spdlog::string_view_t(LoggerImpl::createMessage(tag, fmt, std::forward<Args>(args)...)));
}

template <typename... Args>
void
LoggerImpl::logTrace(std::string_view tag, std::string_view fmt, Args&&... args)
{
    spdlog::default_logger_raw()->log(
            spdlog::level::trace,
            spdlog::string_view_t(LoggerImpl::createMessage(tag, fmt, std::forward<Args>(args)...)));
}

} // namespace utils
