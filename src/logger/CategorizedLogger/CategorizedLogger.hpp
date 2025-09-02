#pragma once

#include <quill/Logger.h>
#include <quill/Backend.h>
#include <quill/Frontend.h>
#include <quill/LogMacros.h>
#include <quill/sinks/FileSink.h>
#include <quill/sinks/ConsoleSink.h>

#include <GenEnum.hpp>

#include "CategorizedLoggerSettings.hpp"

namespace logger {
static constexpr uint8_t kBacktraceDefaultLength = 32;

template <class T, const char* LoggerName, uint8_t BacktraceLength = kBacktraceDefaultLength>
class CategorizedLogger
{
private:
    using Category = T;
    static_assert(Category::getSize() > 0);
    using BaseCategory = typename Category::baseType;

public:
    CategorizedLogger()
    {
        for (BaseCategory i = 0; i < Category::getSize(); ++i)
        {
            // File Sink
            quill::FileSinkConfig cfg;
            cfg.set_open_mode('w');
            cfg.set_filename_append_option(quill::FilenameAppendOption::StartCustomTimestampFormat, kPatternLogFileName);

            auto fileSink =
                quill::Frontend::create_or_get_sink<quill::FileSink>(std::string{kLogSettingsFileName}, std::move(cfg));
            fileSink->set_log_level_filter(getLogLevelByShortName(m_settings.getFileLogLevel(i)));

            // Console Sink
            quill::ConsoleSinkConfig consoleCfg;
            consoleCfg.set_colour_mode(quill::ConsoleSinkConfig::ColourMode::Always);

            auto consoleSink = quill::Frontend::create_or_get_sink<quill::ConsoleSink>(
                std::string{Category::toString(i)}, std::move(consoleCfg));
            consoleSink->set_log_level_filter(getLogLevelByShortName(m_settings.getConsoleLogLevel(i)));

            // Logger create
            m_loggers[i] =
                quill::Frontend::create_or_get_logger(Category::toString(i).data(), {std::move(fileSink), std::move(consoleSink)},
                    quill::PatternFormatterOptions{getPatternFormatter().data(), kPatternFormatterTime.data()});
            m_loggers[i]->init_backtrace(BacktraceLength, quill::LogLevel::Critical);
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

    /**
     * @brief Converts a compile-time integer into a character array representation.
     *
     * This function template takes an integer value specified as a non-type template parameter
     * (`number`) and produces a `std::array<char, N>` containing its decimal representation.
     * The conversion is performed entirely at compile time (`consteval`), making the result
     * usable in constant expressions.
     *
     * @tparam number The integer value to be converted into a character array.
     *
     * @return A `std::array<char, len>` where `len` is the number of decimal digits
     *         in `number`. The array contains the character representation of the number
     *         without a null terminator.
     *
     * @note
     * - The result is not null-terminated. If a C-style string is required,
     *   you should handle null termination manually.
     * - Supports zero and positive integers. Negative values are not supported.
     *
     * @code
     * constexpr auto arr = getNumberAsCharArray<1234>();
     * // arr = {'1','2','3','4'}
     * @endcode
     */
    template <size_t number>
    static consteval auto getNumberAsCharArray()
    {
        constexpr uint8_t kOneDecimalDigit = 10;

        constexpr auto countDigits = [kOneDecimalDigit](size_t x) constexpr
        {
            size_t len = (x <= 0) ? 1 : 0;
            while (x)
            {
                x /= kOneDecimalDigit;
                ++len;
            }
            return len;
        };

        constexpr size_t len = countDigits(number);
        std::array<char, len> result{};

        auto num = number;
        for (int64_t i = len - 1; i >= 0; --i)
        {
            result[i]  = '0' + (num % kOneDecimalDigit);
            num       /= kOneDecimalDigit;
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

        constexpr auto loggerNameLengthAsString = getNumberAsCharArray<size>();

        auto* ptr = res.data();
        ptr       = std::copy(kPatternFormatterLogsPart1.begin(), kPatternFormatterLogsPart1.end(), ptr);
        ptr       = std::copy(kLoggerName.begin(), kLoggerName.end(), ptr);
        ptr       = std::copy(kPatternFormatterLogsPart2.begin(), kPatternFormatterLogsPart2.end(), ptr);
        ptr       = std::copy(std::begin(loggerNameLengthAsString), std::end(loggerNameLengthAsString), ptr);
        ptr       = std::copy(kPatternFormatterLogsPart3.begin(), kPatternFormatterLogsPart3.end(), ptr);

        return res;
    }

    static constexpr std::string_view kPatternLogFileName   = "_%d_%m_%Y_%H_%M_%S";
    static constexpr std::string_view kLogSettingsFileName  = "logs/log.txt";
    static constexpr std::string_view kPatternFormatterTime = "%H:%M:%S.%Qns";

    // clang-format off
    /* Quill log format
     *      [%(time)] [%(thread_id)] [%(short_source_location:^28)] [%(log_level:^11)] [ <LoggerName> ] [%(logger:^<Alignment>) %(message)
     *          - LoggerName - Name of current Categorized Logger
     *          - Alignment  - Logger Category alignment to get more readable logs
     *
     * Example:
     *      [20:45:36.187493109] [27956] [        main.cpp:38         ] [ CRITICAL  ] [ CoreLauncher ] [  Core   ] Core - LOG_CRITICAL
     *      [20:45:36.187493629] [27956] [        main.cpp:44         ] [   INFO    ] [ CoreLauncher ] [ Testing ] Test - LOG_INFO
     */
    // clang-format on
    static constexpr std::string_view kPatternFormatterLogsPart1 =
        "[%(time)] [%(thread_id)] [%(short_source_location:^28)] [%(log_level:^11)] [ ";
    static constexpr std::string_view kPatternFormatterLogsPart2 = " ] [%(logger:^";
    static constexpr std::string_view kPatternFormatterLogsPart3 = ")] %(message)";

    static constexpr std::string_view kLoggerName = LoggerName;

    CategorizedLoggerSettings<T> m_settings;

    std::array<quill::Logger*, Category::getSize()> m_loggers;
};
}  // namespace logger

// clang-format off
#define DEFINE_CAT_LOGGER_MODULE(Name, CategoryType, BacktraceLength)                                      \
    extern const char s_##Name##LoggerName[];                                                              \
    extern logger::CategorizedLogger<CategoryType, s_##Name##LoggerName, BacktraceLength> s_##Name##Logger

#define DEFINE_CAT_LOGGER_MODULE_INITIALIZATION(Name, CategoryType, BacktraceLength)                \
    inline constexpr char s_##Name##LoggerName[] = #Name;                                           \
    logger::CategorizedLogger<CategoryType, s_##Name##LoggerName, BacktraceLength> s_##Name##Logger

#define GET_LOGGER(LoggerName, name, catName) logger::s_##LoggerName##Logger.getLogger(logger::catName::name)

// LOG_INFO
#define CAT_LOG_TRACE_L3(logName, catName, cat, message, ...)  QUILL_LOG_TRACE_L3(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_TRACE_L2(logName, catName, cat, message, ...)  QUILL_LOG_TRACE_L2(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_TRACE_L1(logName, catName, cat, message, ...)  QUILL_LOG_TRACE_L1(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_DEBUG(logName, catName, cat, message, ...)     QUILL_LOG_DEBUG(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_INFO(logName, catName, cat, message, ...)      QUILL_LOG_INFO(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_NOTICE(logName, catName, cat, message, ...)    QUILL_LOG_NOTICE(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
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
#define CAT_LOGV_NOTICE(logName, catName, cat, message, ...)    QUILL_LOGV_NOTICE(GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
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
#define CAT_LOG_NOTICE_LIMIT_TIME(logName, catName, cat, time, message, ...)   QUILL_LOG_NOTICE_LIMIT(time, GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_WARNING_LIMIT_TIME(logName, catName, cat, time, message, ...)  QUILL_LOG_WARNING_LIMIT(time, GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_ERROR_LIMIT_TIME(logName, catName, cat, time, message, ...)    QUILL_LOG_ERROR_LIMIT(time, GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_CRITICAL_LIMIT_TIME(logName, catName, cat, time, message, ...) QUILL_LOG_CRITICAL_LIMIT(time, GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)

// LOG_INFO_LIMIT_EVERY_N
#define CAT_LOG_TRACE_L3_LIMIT_EVERY_N(logName, catName, cat, count, message, ...) QUILL_LOG_TRACE_L3_LIMIT_EVERY_N(count, GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_TRACE_L2_LIMIT_EVERY_N(logName, catName, cat, count, message, ...) QUILL_LOG_TRACE_L2_LIMIT_EVERY_N(count, GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_TRACE_L1_LIMIT_EVERY_N(logName, catName, cat, count, message, ...) QUILL_LOG_TRACE_L1_LIMIT_EVERY_N(count, GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_DEBUG_LIMIT_EVERY_N(logName, catName, cat, count, message, ...)    QUILL_LOG_DEBUG_LIMIT_EVERY_N(count, GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_INFO_LIMIT_EVERY_N(logName, catName, cat, count, message, ...)     QUILL_LOG_INFO_LIMIT_EVERY_N(count, GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_NOTICE_LIMIT_EVERY_N(logName, catName, cat, count, message, ...)   QUILL_LOG_NOTICE_LIMIT_EVERY_N(count, GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_WARNING_LIMIT_EVERY_N(logName, catName, cat, count, message, ...)  QUILL_LOG_WARNING_LIMIT_EVERY_N(count, GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_ERROR_LIMIT_EVERY_N(logName, catName, cat, count, message, ...)    QUILL_LOG_ERROR_LIMIT_EVERY_N(count, GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
#define CAT_LOG_CRITICAL_LIMIT_EVERY_N(logName, catName, cat, count, message, ...) QUILL_LOG_CRITICAL_LIMIT_EVERY_N(count, GET_LOGGER(logName, cat, catName), message, ##__VA_ARGS__)
// clang-format on
