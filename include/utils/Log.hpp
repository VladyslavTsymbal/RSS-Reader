#pragma once

#include "utils/LoggerImpl.hpp"

using utils::LoggerImpl;

#define LOG_FATAL(...) LoggerImpl::getInstance().logFatal(__VA_ARGS__);
#define LOG_ERROR(...) LoggerImpl::getInstance().logError(__VA_ARGS__);
#define LOG_WARN(...) LoggerImpl::getInstance().logWarn(__VA_ARGS__);
#define LOG_INFO(...) LoggerImpl::getInstance().logInfo(__VA_ARGS__);
#define LOG_DEBUG(...) LoggerImpl::getInstance().logDebug(__VA_ARGS__);
#define LOG_TRACE(...) LoggerImpl::getInstance().logTrace(__VA_ARGS__);