find_program(CLANG_FORMAT_EXECUTABLE NAMES clang-format-18)
file(
    GLOB_RECURSE ALL_SOURCE_FILES
    ${CMAKE_SOURCE_DIR}/projects/**/*.c
    ${CMAKE_SOURCE_DIR}/projects/**/*.cpp
    ${CMAKE_SOURCE_DIR}/projects/**/*.h
    ${CMAKE_SOURCE_DIR}/projects/**/*.hpp
)

add_custom_target(
    format
    COMMAND ${CLANG_FORMAT_EXECUTABLE} -i ${ALL_SOURCE_FILES}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)