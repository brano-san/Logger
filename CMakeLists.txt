project ("Logger" CXX)

message(STATUS "Adding library: " ${PROJECT_NAME})
file(GLOB_RECURSE HEADER_FILES "${CMAKE_CURRENT_LIST_DIR}/src/logger/*.hpp")
file(GLOB_RECURSE SOURCE_FILES "${CMAKE_CURRENT_LIST_DIR}/src/logger/*.cpp")

if (ENABLE_DEBUG)
    message(STATUS "Logger debug is Active. Adding debug sources and boost::stacktrace")
    file(GLOB_RECURSE DEBUG_HEADER_FILES "${CMAKE_CURRENT_LIST_DIR}/src/debug/*.hpp")
    file(GLOB_RECURSE DEBUG_SOURCE_FILES "${CMAKE_CURRENT_LIST_DIR}/src/debug/*.cpp")

    list(APPEND HEADER_FILES ${DEBUG_HEADER_FILES})
    list(APPEND SOURCE_FILES ${DEBUG_SOURCE_FILES})
endif()

add_library(${PROJECT_NAME} STATIC ${HEADER_FILES} ${SOURCE_FILES})

set(QUILL_DIR "${CMAKE_CURRENT_LIST_DIR}/quill")
set(QUILL_DISABLE_NON_PREFIXED_MACROS ON)
message(STATUS "Setting quill: " ${QUILL_DIR} "; To target: " ${PROJECT_NAME})
add_subdirectory(${QUILL_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC quill::quill)
target_include_directories(${PROJECT_NAME} PUBLIC "${QUILL_DIR}/include")

if (ENABLE_DEBUG)
    if(WIN32)
        find_package(Boost REQUIRED COMPONENTS stacktrace_windbg)
        target_link_libraries(${PROJECT_NAME} PRIVATE Boost::stacktrace_windbg)
    elseif(UNIX)
        find_package(Boost REQUIRED COMPONENTS stacktrace)
        target_link_libraries(${PROJECT_NAME} PUBLIC Boost::stacktrace)
    endif()
endif()
