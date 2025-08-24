#ifndef LOGGER_CORE_HPP
#define LOGGER_CORE_HPP

#include <quill/Logger.h>
#include <quill/Backend.h>
#include <quill/Frontend.h>
#include <quill/LogMacros.h>
#include <quill/sinks/FileSink.h>
#include <quill/sinks/ConsoleSink.h>

#include "SimpleIni.hpp"

namespace logger {

template <class T, const char* LoggerName>
class CategorizedLogger
{
private:
    using Category = T;
    static_assert(Category::getSize() > 0);
    using BaseCategory = typename Category::baseType;

    struct SinkLogLevel
    {
        std::string_view sink;
        std::string logLevel;
    };

    struct SinksLogLevel
    {
        std::unordered_map<std::string_view, std::string> logLevels = {
            {std::string_view{"File"},    std::string{"T3"}},
            {std::string_view{"Console"}, std::string{"I"} }
        };
    };

public:
    CategorizedLogger()
    {
        loadSettings();

        for (BaseCategory i = 0; i < Category::getSize(); ++i)
        {
            // File Sink
            quill::FileSinkConfig cfg;
            cfg.set_open_mode('w');
            cfg.set_filename_append_option(quill::FilenameAppendOption::StartCustomTimestampFormat, kPatternLogFileName);

            auto fileSink = quill::Frontend::create_or_get_sink<quill::FileSink>(kLogSettingsFileName.data(), std::move(cfg));
            fileSink->set_log_level_filter(getLogLevelByShortName(m_loggerSinks[i].logLevels["File"]));

            // Console Sink
            quill::ConsoleSinkConfig consoleCfg;
            consoleCfg.set_colour_mode(quill::ConsoleSinkConfig::ColourMode::Always);

            auto consoleSink =
                quill::Frontend::create_or_get_sink<quill::ConsoleSink>(Category::toString(i).data(), std::move(consoleCfg));
            consoleSink->set_log_level_filter(getLogLevelByShortName(m_loggerSinks[i].logLevels["Console"]));

            // Logger create
            m_loggers[i] =
                quill::Frontend::create_or_get_logger(Category::toString(i).data(), {std::move(fileSink), std::move(consoleSink)},
                    quill::PatternFormatterOptions{getPatternFormatter().data(), kPatternFormatterTime.data()});
            m_loggers[i]->init_backtrace(32, quill::LogLevel::Critical);
            m_loggers[i]->set_log_level(quill::LogLevel::TraceL3);
        }

        quill::Backend::start();
    }

    quill::Logger* getLogger(const BaseCategory name)
    {
        return m_loggers[name];
    }

    quill::Logger* getFirstLoggerOrNullptr()
    {
        return m_loggers.empty() ? nullptr : m_loggers.front();
    }

private:
    static quill::LogLevel getLogLevelByShortName(std::string_view logLevel)
    {
        quill::BackendOptions opt;
        auto* it = std::ranges::find(opt.log_level_short_codes, logLevel);
        if (it == opt.log_level_short_codes.end())
        {
            return quill::LogLevel::Info;
        }
        return static_cast<quill::LogLevel>(std::distance(opt.log_level_short_codes.begin(), it));
    }

    void loadSettings()
    {
        CSimpleIniA loggerSettingsFile;
        loggerSettingsFile.LoadFile(kLoggerSettingsFileName.data());

        quill::BackendOptions opt;
        static_assert(opt.log_level_descriptions.size() == opt.log_level_short_codes.size());
        for (uint32_t i = 0; i < opt.log_level_short_codes.size(); ++i)
        {
            loggerSettingsFile.SetValue("Description", opt.log_level_descriptions[i].data(), opt.log_level_short_codes[i].data());
        }

        for (BaseCategory i = 0; i < Category::getSize(); ++i)
        {
            for (auto& sink : m_loggerSinks[i].logLevels)
            {
                sink.second = loggerSettingsFile.GetValue(Category::toString(i).data(), sink.first.data(), sink.second.data());
                loggerSettingsFile.SetValue(Category::toString(i).data(), sink.first.data(), sink.second.data());
            }
        }

        loggerSettingsFile.SaveFile(kLoggerSettingsFileName.data());
    }

    template <size_t number>
    static consteval auto getNumberAsCharArray()
    {
        constexpr auto countDigits = [](size_t x) constexpr
        {
            int len = (x <= 0) ? 1 : 0;
            while (x)
            {
                x /= 10;
                ++len;
            }
            return len;
        };

        constexpr int len = countDigits(number);
        std::array<char, len> result{};

        auto num = number;
        for (int i = len - 1; i >= 0; --i)
        {
            result[i]  = '0' + (num % 10);
            num       /= 10;
        }

        return result;
    }

    static consteval auto getPatternFormatter()
    {
        // "+ 2" for whitespaces in begin and end
        constexpr auto size = Category::maxSourceStringLength() + 2;

        // "+ 1" for null-terminated
        std::array<char, kPatternFormatterLogsPart1.size() + kLoggerName.size() + kPatternFormatterLogsPart2.size() + size +
                             kPatternFormatterLogsPart3.size() + 1>
            res{};

        constexpr auto loggerNameLength = getNumberAsCharArray<size>();

        auto* ptr = res.data();
        ptr       = std::copy(kPatternFormatterLogsPart1.begin(), kPatternFormatterLogsPart1.end(), ptr);
        ptr       = std::copy(kLoggerName.begin(), kLoggerName.end(), ptr);
        ptr       = std::copy(kPatternFormatterLogsPart2.begin(), kPatternFormatterLogsPart2.end(), ptr);
        ptr       = std::copy(std::begin(loggerNameLength), std::end(loggerNameLength), ptr);
        ptr       = std::copy(kPatternFormatterLogsPart3.begin(), kPatternFormatterLogsPart3.end(), ptr);

        return res;
    }

    static constexpr std::string_view kLoggerSettingsFileName = "LogSettings.ini";

    static constexpr std::string_view kPatternLogFileName   = "_%d_%m_%Y_%H_%M_%S";
    static constexpr std::string_view kLogSettingsFileName  = "logs/log.txt";
    static constexpr std::string_view kPatternFormatterTime = "%H:%M:%S.%Qns";

    static constexpr std::string_view kPatternFormatterLogsPart1 =
        "[%(time)] [%(thread_id)] [%(short_source_location:^28)] [%(log_level:^11)] [ ";
    static constexpr std::string_view kPatternFormatterLogsPart2 = " ] [%(logger:^";
    static constexpr std::string_view kPatternFormatterLogsPart3 = ")] %(message)";

    static constexpr std::string_view kLoggerName = LoggerName;

    std::array<quill::Logger*, Category::getSize()> m_loggers;
    std::array<SinksLogLevel, Category::getSize()> m_loggerSinks;
};
}  // namespace logger

// clang-format off
#define DEFINE_CAT_LOGGER_MODULE(Name, CategoryType) \
    extern const char s_##Name##LoggerName[]; \
    extern logger::CategorizedLogger<CategoryType, s_##Name##LoggerName> s_##Name##Logger

#define DEFINE_CAT_LOGGER_MODULE_INITIALIZATION(Name, CategoryType) \
    inline constexpr char s_##Name##LoggerName[] = #Name; \
    logger::CategorizedLogger<CategoryType, s_##Name##LoggerName> s_##Name##Logger

#define GET_LOGGER(LoggerName, name, catName) logger::s_##LoggerName##Logger.getLogger(logger::catName::name)

// LOG_INFO
#define CAT_LOG_TRACE_L3(logName, catName, cat, message, ...)  QUILL_LOG_TRACE_L3(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_TRACE_L2(logName, catName, cat, message, ...)  QUILL_LOG_TRACE_L2(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_TRACE_L1(logName, catName, cat, message, ...)  QUILL_LOG_TRACE_L1(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_DEBUG(logName, catName, cat, message, ...)     QUILL_LOG_DEBUG(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_INFO(logName, catName, cat, message, ...)      QUILL_LOG_INFO(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_WARNING(logName, catName, cat, message, ...)   QUILL_LOG_WARNING(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_ERROR(logName, catName, cat, message, ...)     QUILL_LOG_ERROR(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_CRITICAL(logName, catName, cat, message, ...)  QUILL_LOG_CRITICAL(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_BACKTRACE(logName, catName, cat, message, ...) QUILL_LOG_BACKTRACE(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)

// LOGV_INFO
#define CAT_LOGV_TRACE_L3(logName, catName, cat, message, ...)  QUILL_LOGV_TRACE_L3(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOGV_TRACE_L2(logName, catName, cat, message, ...)  QUILL_LOGV_TRACE_L2(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOGV_TRACE_L1(logName, catName, cat, message, ...)  QUILL_LOGV_TRACE_L1(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOGV_DEBUG(logName, catName, cat, message, ...)     QUILL_LOGV_DEBUG(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOGV_INFO(logName, catName, cat, message, ...)      QUILL_LOGV_INFO(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOGV_WARNING(logName, catName, cat, message, ...)   QUILL_LOGV_WARNING(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOGV_ERROR(logName, catName, cat, message, ...)     QUILL_LOGV_ERROR(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOGV_CRITICAL(logName, catName, cat, message, ...)  QUILL_LOGV_CRITICAL(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOGV_BACKTRACE(logName, catName, cat, message, ...) QUILL_LOGV_BACKTRACE(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)

// LOG_INFO_LIMIT
#define CAT_LOG_TRACE_L3_LIMIT_TIME(logName, catName, cat, time, message, ...) QUILL_LOG_TRACE_L3_LIMIT(time, GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_TRACE_L2_LIMIT_TIME(logName, catName, cat, time, message, ...) QUILL_LOG_TRACE_L2_LIMIT(time, GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_TRACE_L1_LIMIT_TIME(logName, catName, cat, time, message, ...) QUILL_LOG_TRACE_L1_LIMIT(time, GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_DEBUG_LIMIT_TIME(logName, catName, cat, time, message, ...)    QUILL_LOG_DEBUG_LIMIT(time, GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_INFO_LIMIT_TIME(logName, catName, cat, time, message, ...)     QUILL_LOG_INFO_LIMIT(time, GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_WARNING_LIMIT_TIME(logName, catName, cat, time, message, ...)  QUILL_LOG_WARNING_LIMIT(time, GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_ERROR_LIMIT_TIME(logName, catName, cat, time, message, ...)    QUILL_LOG_ERROR_LIMIT(time, GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_CRITICAL_LIMIT_TIME(logName, catName, cat, time, message, ...) QUILL_LOG_CRITICAL_LIMIT(time, GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)

// LOG_INFO_LIMIT_EVERY_N
#define CAT_LOG_TRACE_L3_LIMIT_EVERY_N(logName, catName, cat, count, message, ...) QUILL_LOG_TRACE_L3_LIMIT_EVERY_N(count, GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_TRACE_L2_LIMIT_EVERY_N(logName, catName, cat, count, message, ...) QUILL_LOG_TRACE_L2_LIMIT_EVERY_N(count, GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_TRACE_L1_LIMIT_EVERY_N(logName, catName, cat, count, message, ...) QUILL_LOG_TRACE_L1_LIMIT_EVERY_N(count, GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_DEBUG_LIMIT_EVERY_N(logName, catName, cat, count, message, ...)    QUILL_LOG_DEBUG_LIMIT_EVERY_N(count, GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_INFO_LIMIT_EVERY_N(logName, catName, cat, count, message, ...)     QUILL_LOG_INFO_LIMIT_EVERY_N(count, GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_WARNING_LIMIT_EVERY_N(logName, catName, cat, count, message, ...)  QUILL_LOG_WARNING_LIMIT_EVERY_N(count, GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_ERROR_LIMIT_EVERY_N(logName, catName, cat, count, message, ...)    QUILL_LOG_ERROR_LIMIT_EVERY_N(count, GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_CRITICAL_LIMIT_EVERY_N(logName, catName, cat, count, message, ...) QUILL_LOG_CRITICAL_LIMIT_EVERY_N(count, GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
// clang-format on

#endif  // LOGGER_CORE_HPP
