#include "StackTrace.hpp"

#include "quill/Logger.h"
#include "quill/Frontend.h"
#include "quill/sinks/ConsoleSink.h"

#include "../logger/CategorizedLogger.hpp"

#include <boost/stacktrace.hpp>
#include <iostream>

static constexpr std::string_view kStackTraceLoggerName = "StackTrace";

auto s_crashLogger = quill::Frontend::create_or_get_logger(kStackTraceLoggerName.data(),
    quill::Frontend::create_or_get_sink<quill::ConsoleSink>(kStackTraceLoggerName.data()),
    quill::PatternFormatterOptions{logger::kPatternFormatterLogs.data(), logger::kPatternFormatterTime.data()});

#if defined(_WIN32) || defined(_WIN64)

LONG WINAPI unhandledExceptionFilter(_EXCEPTION_POINTERS* ExceptionInfo)
{
    QUILL_LOG_CRITICAL(s_crashLogger, "CRASH");
    std::cerr << boost::stacktrace::stacktrace();
    return 0;
}

#endif

void debug::setStackTraceOutputOnCrash()
{
    boost::stacktrace::this_thread::set_capture_stacktraces_at_throw(true);
#ifdef __linux__
    // No realization yet
#else
    SetUnhandledExceptionFilter(unhandledExceptionFilter);
    AddVectoredContinueHandler(0, unhandledExceptionFilter);

    std::set_terminate(
        []()
        {
            QUILL_LOG_CRITICAL(s_crashLogger, "CRASH");
            std::cerr << boost::stacktrace::stacktrace();
            exit(-1);
        });
#endif
}
