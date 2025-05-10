#include "StackTrace.hpp"

#include <boost/stacktrace.hpp>
#include <logger/CategorizedLogger.hpp>

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#endif

quill::Logger* s_crashLogger;

void* getDLPointer(const void* address) noexcept
{
#if defined(__linux__)
    Dl_info dlInfo;
    int result = dladdr(address, &dlInfo);
    if (result != 0)
    {
        address = (void*)((char*)address - (char*)dlInfo.dli_fbase);
    }
#endif
    return (void*)address;
}

std::string getStackTraceAsFormattedString()
{
    auto st = boost::stacktrace::stacktrace().as_vector();
    std::string log;
    for (const auto& frame : st)
    {
        std::stringstream s;
        s << getDLPointer(frame.address());
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

#if defined(__linux__)

void signalHandler(int signum)
{
    QUILL_LOG_CRITICAL(s_crashLogger, "CRASH {}", getStackTraceAsFormattedString());
    exit(-1);
}

void setupSignals()
{
    signal(SIGSEGV, signalHandler);
    signal(SIGABRT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGFPE, signalHandler);
    signal(SIGINT, signalHandler);
    signal(SIGHUP, signalHandler);
    signal(SIGQUIT, signalHandler);
    signal(SIGBUS, signalHandler);
    signal(SIGSTKFLT, signalHandler);
}

#endif

void debug::setStackTraceOutputOnCrash(quill::Logger* logger)
{
    s_crashLogger = logger;

    // if this line fails compile, then you probably should build boost with backtrace extension
    // boost from apt doesn't have backtrace support
    boost::stacktrace::this_thread::set_capture_stacktraces_at_throw(true);

#if defined(__linux__)
    setupSignals();

    std::set_terminate(
        []()
        {
            QUILL_LOG_CRITICAL(s_crashLogger, "Crash {}", getStackTraceAsFormattedString());
            exit(-1);
        });
#elif defined(_WIN32) || defined(_WIN64)
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
