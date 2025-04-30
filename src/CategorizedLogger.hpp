#ifndef LOGGER_CORE_HPP
#define LOGGER_CORE_HPP

#include "quill/Backend.h"
#include "quill/Frontend.h"
#include "quill/LogMacros.h"
#include "quill/Logger.h"

#include "quill/sinks/ConsoleSink.h"
#include "quill/sinks/RotatingFileSink.h"

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

        // File loggers
        for (BaseCategory i = 0; i < Category::kCount; ++i)
        {
            std::vector<std::shared_ptr<quill::Sink>> sinks;

            // auto sink = quill::Frontend::create_or_get_sink<quill::RotatingFileSink>("log.txt",
            //    []()
            //    {
            //        quill::RotatingFileSinkConfig cfg;
            //        cfg.set_rotation_max_file_size(1024);
            //        return cfg;
            //    }());

            auto sink = quill::Frontend::create_or_get_sink<quill::FileSink>("log.txt",
                []()
                {
                    quill::FileSinkConfig cfg;
                    cfg.set_open_mode('w');
                    cfg.set_filename_append_option(quill::FilenameAppendOption::StartCustomTimestampFormat, "_%d_%m_%Y_%H_%M_%S");
                    return cfg;
                }());
            sinks.push_back(sink);

            auto consoleSink = quill::Frontend::create_or_get_sink<quill::ConsoleSink>(Category::to_string(i).data());
            sinks.push_back(consoleSink);

            m_loggers[i] = quill::Frontend::create_or_get_logger(Category::to_string(i).data(), std::move(sinks),
                quill::PatternFormatterOptions{kPatternFormatterLogs.data(), kPatternFormatterTime.data()});

            m_loggers[i]->init_backtrace(32, quill::LogLevel::Error);
            m_loggers[i]->set_log_level(quill::LogLevel::TraceL3);
        }
    }

    quill::Logger* getLogger(const BaseCategory name)
    {
        return m_loggers[name];
    }

private:
    static constexpr std::string_view kPatternFormatterTime = "%H:%M:%S.%Qns";
    static constexpr std::string_view kPatternFormatterLogs =
        "[%(time)] [%(thread_id)] [%(short_source_location:^28)] [%(log_level:^11)] [%(logger:^6)] %(message)";
    // TODO: Динамическое определение длины метаданных для уровня названия логгера___^^^^^^^^^^^^

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
