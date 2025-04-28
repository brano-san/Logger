#ifndef LOGGER_CORE_HPP
#define LOGGER_CORE_HPP

#include "quill/Backend.h"
#include "quill/Frontend.h"
#include "quill/LogMacros.h"
#include "quill/Logger.h"
#include "quill/sinks/ConsoleSink.h"

namespace logger {

enum LoggerName : uint8_t
{
    Core  = 0,
    Count = 255
};

inline std::string toString(LoggerName name)
{
    switch (name)
    {
    case LoggerName::Core: return "Core";
    case LoggerName::Count: return "Unknown";
    }
    return "Unknown";
}

class Logger
{
public:
    Logger()
    {
        quill::Backend::start();

        for (uint8_t i = 0; i < LoggerName::Count; ++i)
        {
            s_loggers[i] = quill::Frontend::create_or_get_logger(toString(static_cast<LoggerName>(i)),
                quill::Frontend::create_or_get_sink<quill::ConsoleSink>(toString(static_cast<LoggerName>(i))));
            s_loggers[i]->init_backtrace(32);
            s_loggers[i]->set_log_level(quill::LogLevel::TraceL3);
        }
    }

    quill::Logger* getLogger(const LoggerName name)
    {
        return s_loggers[name];
    }

private:
    std::array<quill::Logger*, LoggerName::Count> s_loggers;
};

static Logger s_logger;
}  // namespace logger

#define LOG_TRACE_L3(name, message, ...) \
    QUILL_LOG_TRACE_L3(logger::s_logger.getLogger(logger::LoggerName::name), message, ##__VA_ARGS__)
#define LOG_TRACE_L2(name, message, ...) \
    QUILL_LOG_TRACE_L2(logger::s_logger.getLogger(logger::LoggerName::name), message, ##__VA_ARGS__)
#define LOG_TRACE_L1(name, message, ...) \
    QUILL_LOG_TRACE_L1(logger::s_logger.getLogger(logger::LoggerName::name), message, ##__VA_ARGS__)
#define LOG_DEBUG(name, message, ...) \
    QUILL_LOG_DEBUG(logger::s_logger.getLogger(logger::LoggerName::name), message, ##__VA_ARGS__)
#define LOG_INFO(name, message, ...) QUILL_LOG_INFO(logger::s_logger.getLogger(logger::LoggerName::name), message, ##__VA_ARGS__)
#define LOG_WARNING(name, message, ...) \
    QUILL_LOG_WARNING(logger::s_logger.getLogger(logger::LoggerName::name), message, ##__VA_ARGS__)
#define LOG_ERROR(name, message, ...) \
    QUILL_LOG_ERROR(logger::s_logger.getLogger(logger::LoggerName::name), message, ##__VA_ARGS__)
#define LOG_CRITICAL(name, message, ...) \
    QUILL_LOG_CRITICAL(logger::s_logger.getLogger(logger::LoggerName::name), message, ##__VA_ARGS__)

#endif  // LOGGER_CORE_HPP
