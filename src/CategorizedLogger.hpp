#ifndef LOGGER_CORE_HPP
#define LOGGER_CORE_HPP

#include "quill/Backend.h"
#include "quill/Frontend.h"
#include "quill/LogMacros.h"
#include "quill/Logger.h"
#include "quill/sinks/ConsoleSink.h"

namespace logger {

template <class T>
class CategorizedLogger
{
private:
    using Category     = T;
    using BaseCategory = typename Category::baseType;

public:
    CategorizedLogger()
    {
        quill::Backend::start();

        for (BaseCategory i = 0; i < Category::kCount; ++i)
        {
            m_loggers[i] = quill::Frontend::create_or_get_logger(Category::to_string(i).data(),
                quill::Frontend::create_or_get_sink<quill::ConsoleSink>(Category::to_string(i).data()));
            m_loggers[i]->init_backtrace(32, quill::LogLevel::Error);
            m_loggers[i]->set_log_level(quill::LogLevel::TraceL3);
        }
    }

    quill::Logger* getLogger(const BaseCategory name)
    {
        return m_loggers[name];
    }

private:
    std::array<quill::Logger*, Category::kCount> m_loggers;
};
}  // namespace logger

#define DEFINE_CAT_LOGGER_MODULE(Name, CategoryType) extern logger::CategorizedLogger<CategoryType> s_##Name##Logger
#define DEFINE_CAT_LOGGER_MODULE_INITIALIZATION(Name, CategoryType) logger::CategorizedLogger<CategoryType> s_##Name##Logger

#define GET_LOGGER(LoggerName, name, catName) logger::s_##LoggerName##Logger.getLogger(logger::catName::k##name)

#define CAT_LOG_TRACE_L3(logName, catName, cat, message, ...) \
    QUILL_LOG_TRACE_L3(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_TRACE_L2(logName, catName, cat, message, ...) \
    QUILL_LOG_TRACE_L2(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_TRACE_L1(logName, catName, cat, message, ...) \
    QUILL_LOG_TRACE_L1(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_DEBUG(logName, catName, cat, message, ...) \
    QUILL_LOG_DEBUG(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_INFO(logName, catName, cat, message, ...) \
    QUILL_LOG_INFO(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_WARNING(logName, catName, cat, message, ...) \
    QUILL_LOG_WARNING(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_ERROR(logName, catName, cat, message, ...) \
    QUILL_LOG_ERROR(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_CRITICAL(logName, catName, cat, message, ...) \
    QUILL_LOG_CRITICAL(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)

#endif  // LOGGER_CORE_HPP
