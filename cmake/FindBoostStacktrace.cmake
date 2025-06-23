function(FindAndLinkBoost)
    if (NOT ENABLE_DEBUG)
        message(STATUS "Logger debug feature is disabled. Skip boost stacktrace search")
        return()
    endif()

    if(WIN32)

        find_package(Boost COMPONENTS stacktrace_windbg)
        if (Boost_STACKTRACE_WINDBG_FOUND)
            message(STATUS "Found Boost::stacktrace with windbg backend")
            target_link_libraries(${PROJECT_NAME} PRIVATE Boost::stacktrace_windbg)
            target_compile_definitions(${PROJECT_NAME} PRIVATE BOOST_STACKTRACE_USE_WINDBG)
            return()
        endif()

    elseif(UNIX)

        cmake_policy(SET CMP0167 OLD)
        find_package(Boost COMPONENTS stacktrace_backtrace)
        if (Boost_STACKTRACE_BACKTRACE_FOUND)
            message(STATUS "Found Boost::stacktrace with backtrace backend")
            target_link_libraries(${PROJECT_NAME} PRIVATE Boost::stacktrace_backtrace)
            target_compile_definitions(${PROJECT_NAME} PRIVATE BOOST_STACKTRACE_USE_BACKTRACE)
            return()
        endif()

    endif()

    find_package(Boost REQUIRED COMPONENTS stacktrace_noop)
    message(WARNING "Found Boost::stacktrace without backend. Stacktrace would not show any trace")
    target_link_libraries(${PROJECT_NAME} PRIVATE Boost::stacktrace_noop)
endfunction()
