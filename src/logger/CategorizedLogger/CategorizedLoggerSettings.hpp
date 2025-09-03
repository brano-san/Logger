#pragma once

#include <cstdint>
#include <GenEnum.hpp>

#include <quill/Logger.h>
#include <quill/Backend.h>
#include <quill/Frontend.h>
#include <quill/LogMacros.h>
#include <quill/sinks/FileSink.h>
#include <quill/sinks/ConsoleSink.h>

#include "SimpleIni.hpp"

namespace logger {
template <class T>
class CategorizedLoggerSettings
{
private:
    using Category = T;
    static_assert(Category::getSize() > 0);
    using BaseCategory = typename Category::baseType;

    struct SinksLogLevel
    {
        GENENUM(uint8_t, LogSource, File, Console);
        GENENUM(uint8_t, LogLevel, T3, T2, T1, D, I, N, W, E, C, BT, _);  // From quill library

        struct CurrentAndDefualtLogLevel
        {
            LogLevel currentLogLevel;
            LogLevel defaultLogLevel;
        };

        std::map<LogSource, CurrentAndDefualtLogLevel> logLevels = {
            {LogSources::File,    CurrentAndDefualtLogLevel{LogLevels::T3, LogLevels::T3}},
            {LogSources::Console, CurrentAndDefualtLogLevel{LogLevels::I, LogLevels::I}  }
        };
    };

public:
    CategorizedLoggerSettings() noexcept
    {
        try
        {
            loadSettings();
        }
        catch (const std::exception& ex)
        {
            std::printf("Got exception during initialize categorized logger settings");
        }
    }

    std::string_view getFileLogLevel(BaseCategory category)
    {
        const auto fileLogLevel = m_loggerSinks[category].logLevels[SinksLogLevel::LogSources::File];
        return SinksLogLevel::LogLevels::toString(fileLogLevel.currentLogLevel);
    }

    std::string_view getConsoleLogLevel(BaseCategory category)
    {
        const auto consoleLogLevel = m_loggerSinks[category].logLevels[SinksLogLevel::LogSources::Console];
        return SinksLogLevel::LogLevels::toString(consoleLogLevel.currentLogLevel);
    }

private:
    void loadSettings()
    {
        CSimpleIniA loggerSettingsFile;
        loggerSettingsFile.LoadFile(std::string{kLoggerSettingsFileName}.data());

        quill::BackendOptions opt;
        static_assert(opt.log_level_descriptions.size() == opt.log_level_short_codes.size());
        for (uint32_t i = 0; i < opt.log_level_short_codes.size(); ++i)
        {
            loggerSettingsFile.SetValue(std::string{kDescriptionSettingsCategoryName}.data(),
                opt.log_level_descriptions.at(i).data(), opt.log_level_short_codes.at(i).data());
        }

        for (BaseCategory i = 0; i < Category::getSize(); ++i)
        {
            for (auto& sink : m_loggerSinks[i].logLevels)
            {
                const auto logSource = SinksLogLevel::LogSources::toString(sink.first);
                const auto logLevel  = SinksLogLevel::LogLevels::toString(sink.second.currentLogLevel);

                const auto levelFromSettings =
                    loggerSettingsFile.GetValue(Category::toString(i).data(), logSource.data(), logLevel.data());

                const bool result = SinksLogLevel::LogLevels::fromString(levelFromSettings, sink.second.currentLogLevel);
                if (!result)
                {
                    sink.second.currentLogLevel = sink.second.defaultLogLevel;
                }

                const auto newLogLevel = SinksLogLevel::LogLevels::toString(sink.second.currentLogLevel);
                loggerSettingsFile.SetValue(Category::toString(i).data(), logSource.data(), newLogLevel.data());
            }
        }

        loggerSettingsFile.SaveFile(std::string{kLoggerSettingsFileName}.data());
    }

    static constexpr std::string_view kLoggerSettingsFileName          = "LogSettings.ini";
    static constexpr std::string_view kDescriptionSettingsCategoryName = "Description";

    std::array<SinksLogLevel, Category::getSize()> m_loggerSinks;
};
}  // namespace logger
