#include "StackTrace.hpp"

#include "quill/Logger.h"

#include <boost/stacktrace.hpp>

quill::Logger* s_crashLogger;

std::string getStackTraceAsFormattedString()
{
    auto st = boost::stacktrace::stacktrace().as_vector();
    std::string log;
    for (const auto& frame : st)
    {
        std::stringstream s;
        s << frame.address();
        log += "Address[" + s.str() + "] Location[" + boost::stacktrace::to_string(frame) + "]";

        if (!frame.source_file().empty())
        {
            log += " FuncName[" + frame.source_file() + "]";
        }

        log += '\n';
    }
    return log;
}

#if defined(_WIN32) || defined(_WIN64)

LONG WINAPI unhandledExceptionFilter(_EXCEPTION_POINTERS* ExceptionInfo)
{
    QUILL_LOG_CRITICAL(s_crashLogger, "CRASH {}", getStackTraceAsFormattedString());
    return 0;
}

#endif

void debug::setStackTraceOutputOnCrash(quill::Logger* logger)
{
    s_crashLogger = logger;
    boost::stacktrace::this_thread::set_capture_stacktraces_at_throw(true);
#ifdef __linux__
    // No realization yet
#else
    SetUnhandledExceptionFilter(unhandledExceptionFilter);
    AddVectoredContinueHandler(0, unhandledExceptionFilter);

    std::set_terminate(
        []()
        {
            QUILL_LOG_CRITICAL(s_crashLogger, "Crash {}", getStackTraceAsFormattedString());
            exit(-1);
        });
#endif
}
