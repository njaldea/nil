find_program(CLANG_FORMAT_EXECUTABLE NAMES clang-format-18)
file(
    GLOB_RECURSE ALL_SOURCE_FILES
    ${CMAKE_SOURCE_DIR}/projects/**/*.c
    ${CMAKE_SOURCE_DIR}/projects/**/*.cpp
    ${CMAKE_SOURCE_DIR}/projects/**/*.h
    ${CMAKE_SOURCE_DIR}/projects/**/*.hpp
    ${CMAKE_SOURCE_DIR}/sandbox/**/*.c
    ${CMAKE_SOURCE_DIR}/sandbox/**/*.cpp
    ${CMAKE_SOURCE_DIR}/sandbox/**/*.h
    ${CMAKE_SOURCE_DIR}/sandbox/**/*.hpp
)

add_custom_target(
    format
    COMMAND ${CLANG_FORMAT_EXECUTABLE} -i ${ALL_SOURCE_FILES}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

set(ENABLE_CLANG_TIDY OFF CACHE BOOL "[0 | OFF - 1 | ON]: build with clang-tidy?")
if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy-18 REQUIRED)
    set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_EXECUTABLE})
endif()
