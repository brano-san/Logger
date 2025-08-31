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
add_library(Logger::Logger ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src")

if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX)
endif()

include(cmake/FindBoostStacktrace.cmake)
FindAndLinkBoost()

set(QUILL_DIR "${CMAKE_CURRENT_LIST_DIR}/third_party/quill")
set(QUILL_DISABLE_NON_PREFIXED_MACROS ON)
message(STATUS "Setting quill: " ${QUILL_DIR} "; To target: " ${PROJECT_NAME})
add_subdirectory(${QUILL_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC quill::quill)
target_include_directories(${PROJECT_NAME} PUBLIC "${QUILL_DIR}/include")

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/third_party/GenEnum")
target_link_libraries(${PROJECT_NAME} PUBLIC GenEnum::GenEnum)
