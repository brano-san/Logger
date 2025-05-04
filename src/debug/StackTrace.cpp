#include "StackTrace.hpp"

#include "quill/Logger.h"
#include "quill/Frontend.h"
#include "quill/sinks/ConsoleSink.h"

#include "../logger/CategorizedLogger.hpp"

static constexpr std::string_view kStackTraceLoggerName = "StackTrace";

auto s_crashLogger = quill::Frontend::create_or_get_logger(kStackTraceLoggerName.data(),
    quill::Frontend::create_or_get_sink<quill::ConsoleSink>(kStackTraceLoggerName.data()),
    quill::PatternFormatterOptions{logger::kPatternFormatterLogs.data(), logger::kPatternFormatterTime.data()});

#if defined(_WIN32) || defined(_WIN64)

LONG WINAPI unhandledExceptionFilter(_EXCEPTION_POINTERS* ExceptionInfo)
{
    QUILL_LOG_CRITICAL(s_crashLogger, "CRASH");
    return 0;
}

#endif

void debug::setStackTraceOutputOnCrash()
{
#ifdef __linux__
    // No realization yet
#else
    SetUnhandledExceptionFilter(unhandledExceptionFilter);
    AddVectoredContinueHandler(0, unhandledExceptionFilter);

    std::set_terminate(
        []()
        {
            QUILL_LOG_CRITICAL(s_crashLogger, "CRASH");
            exit(-1);
        });
#endif
}
