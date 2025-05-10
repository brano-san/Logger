#ifndef LOGGER_STACK_TRACE_HPP
#define LOGGER_STACK_TRACE_HPP

#include <quill/Logger.h>

namespace debug {
void setStackTraceOutputOnCrash(quill::Logger* logger);
}  // namespace debug

#endif  // LOGGER_STACK_TRACE_HPP
