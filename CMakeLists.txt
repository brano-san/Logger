project ("Logger" CXX)

message(STATUS "Adding library: " ${PROJECT_NAME})
file(GLOB_RECURSE HEADER_FILES ${CMAKE_CURRENT_LIST_DIR}/src/*.hpp)
file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
add_library (${PROJECT_NAME} STATIC ${HEADER_FILES} ${SOURCE_FILES})

set(QUILL_DIR "${CMAKE_CURRENT_LIST_DIR}/quill")
set(QUILL_DISABLE_NON_PREFIXED_MACROS ON)
message(STATUS "Setting quill: " ${QUILL_DIR} "; To target: " ${PROJECT_NAME})
add_subdirectory (${QUILL_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC quill::quill)
target_include_directories(${PROJECT_NAME} PUBLIC "${QUILL_DIR}/include")