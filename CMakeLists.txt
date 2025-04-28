project ("Logger" CXX)

file(GLOB_RECURSE HEADER_FILES ${CMAKE_CURRENT_LIST_DIR}/src/*.hpp)
file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
add_library (${PROJECT_NAME} STATIC ${HEADER_FILES} ${SOURCE_FILES})

set(QUILL_DIR "${CMAKE_CURRENT_LIST_DIR}/quill")
set(QUILL_DISABLE_NON_PREFIXED_MACROS ON)
add_subdirectory (${QUILL_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC quill)
target_include_directories(${PROJECT_NAME} PRIVATE "${QUILL_DIR}/include")