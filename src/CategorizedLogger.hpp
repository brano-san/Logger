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

class CategorizedLogger
{
public:
    CategorizedLogger()
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
}  // namespace logger

#define DEFINE_CAT_LOGGER_MODULE(LoggerName) static logger::CategorizedLogger s_##LoggerName##Logger

#define GET_LOGGER(LoggerName, name) logger::s_##LoggerName##Logger.getLogger(logger::name)

#endif  // LOGGER_CORE_HPP
