#ifndef LOGGER_CORE_HPP
#define LOGGER_CORE_HPP

#include <print>

#include <quill/Logger.h>
#include <quill/Backend.h>
#include <quill/Frontend.h>
#include <quill/LogMacros.h>
#include <quill/sinks/FileSink.h>
#include <quill/sinks/ConsoleSink.h>

#include "SimpleIni.hpp"

namespace logger {

template <class T>
class CategorizedLogger
{
private:
    using Category     = T;
    using BaseCategory = typename Category::baseType;

    struct SinkLogLevel
    {
        std::string_view sink;
        std::string logLevel;
    };

    struct SinksLogLevel
    {
        std::unordered_map<std::string_view, std::string> logLevels = {
            {std::string_view{"File"}, std::string{"T3"}},
            {std::string_view{"Console"}, std::string{"I"}}
        };
    };

public:
    CategorizedLogger()
    {
        loadSettings();

        quill::Backend::start();

        for (BaseCategory i = 0; i < Category::kCount; ++i)
        {
            std::vector<std::shared_ptr<quill::Sink>> sinks;

            // File Sink
            quill::FileSinkConfig cfg;
            cfg.set_open_mode('w');
            cfg.set_filename_append_option(quill::FilenameAppendOption::StartCustomTimestampFormat, "_%d_%m_%Y_%H_%M_%S");

            auto fileSink = quill::Frontend::create_or_get_sink<quill::FileSink>("logs/log.txt", std::move(cfg));
            fileSink->set_log_level_filter(getLogLevelByShortName(s_loggerSinks[i].logLevels["File"]));
            sinks.push_back(fileSink);

            // Console Sink
            auto consoleSink = quill::Frontend::create_or_get_sink<quill::ConsoleSink>(Category::to_string(i).data());
            consoleSink->set_log_level_filter(getLogLevelByShortName(s_loggerSinks[i].logLevels["Console"]));
            sinks.push_back(consoleSink);

            // Logger create
            m_loggers[i] = quill::Frontend::create_or_get_logger(Category::to_string(i).data(), std::move(sinks),
                quill::PatternFormatterOptions{kPatternFormatterLogs.data(), kPatternFormatterTime.data()});
            m_loggers[i]->init_backtrace(32, quill::LogLevel::Critical);
            m_loggers[i]->set_log_level(quill::LogLevel::TraceL3);
        }
    }

    quill::Logger* getLogger(const BaseCategory name)
    {
        return m_loggers[name];
    }

private:
    static void loadSettings()
    {
        CSimpleIniA loggerSettingsFile;
        loggerSettingsFile.LoadFile(kLoggerSettingsFileName.data());

        quill::BackendOptions opt;
        static_assert(opt.log_level_descriptions.size() == opt.log_level_short_codes.size());
        for (uint32_t i = 0; i < opt.log_level_short_codes.size(); ++i)
        {
            loggerSettingsFile.SetValue("Description", opt.log_level_descriptions[i].data(), opt.log_level_short_codes[i].data());
        }

        for (BaseCategory i = 0; i < Category::kCount; ++i)
        {
            for (auto& sink : s_loggerSinks[i].logLevels)
            {
                sink.second = loggerSettingsFile.GetValue(Category::to_string(i).data(), sink.first.data(), sink.second.data());
                loggerSettingsFile.SetValue(Category::to_string(i).data(), sink.first.data(), sink.second.data());
            }
        }

        loggerSettingsFile.SaveFile(kLoggerSettingsFileName.data());
    }

    static quill::LogLevel getLogLevelByShortName(std::string_view logLevel)
    {
        quill::BackendOptions opt;
        auto it = std::find(opt.log_level_short_codes.begin(), opt.log_level_short_codes.end(), logLevel);
        if (it == opt.log_level_short_codes.end())
        {
            std::println("Cannot find log level by given short name - {}. Return INFO", logLevel);
            return quill::LogLevel::Info;
        }
        return static_cast<quill::LogLevel>(std::distance(opt.log_level_short_codes.begin(), it));
    }

    static constexpr std::string_view kLoggerSettingsFileName = "LogSettings.ini";

    static constexpr std::string_view kPatternFormatterTime = "%H:%M:%S.%Qns";
    static constexpr std::string_view kPatternFormatterLogs =
        "[%(time)] [%(thread_id)] [%(short_source_location:^28)] [%(log_level:^11)] [%(logger:^6)] %(message)";

    inline static std::array<SinksLogLevel, Category::kCount> s_loggerSinks;

    std::array<quill::Logger*, Category::kCount> m_loggers;
};
}  // namespace logger

#define DEFINE_CAT_LOGGER_MODULE(Name, CategoryType) extern logger::CategorizedLogger<CategoryType> s_##Name##Logger
#define DEFINE_CAT_LOGGER_MODULE_INITIALIZATION(Name, CategoryType) logger::CategorizedLogger<CategoryType> s_##Name##Logger

#define GET_LOGGER(LoggerName, name, catName) logger::s_##LoggerName##Logger.getLogger(logger::catName::k##name)

#define CAT_LOG_TRACE_L3(logName, catName, cat, message, ...) QUILL_LOG_TRACE_L3(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_TRACE_L2(logName, catName, cat, message, ...) QUILL_LOG_TRACE_L2(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_TRACE_L1(logName, catName, cat, message, ...) QUILL_LOG_TRACE_L1(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_DEBUG(logName, catName, cat, message, ...)    QUILL_LOG_DEBUG(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_INFO(logName, catName, cat, message, ...)     QUILL_LOG_INFO(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_WARNING(logName, catName, cat, message, ...)  QUILL_LOG_WARNING(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_ERROR(logName, catName, cat, message, ...)    QUILL_LOG_ERROR(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_CRITICAL(logName, catName, cat, message, ...) QUILL_LOG_CRITICAL(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)

#endif  // LOGGER_CORE_HPP
