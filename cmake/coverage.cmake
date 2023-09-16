set(ENABLE_COVERAGE OFF CACHE BOOL "[0 | OFF - 1 | ON]: build with coverage?")

if(NOT ENABLE_COVERAGE)
    return()
endif()

find_program(LCOV_EXECUTABLE NAMES lcov)
find_program(GENHTML_EXECUTABLE NAMES genhtml)
find_program(NPX_EXECUTABLE NAMES npx)

set(COVERAGE_DIR ${CMAKE_BINARY_DIR}/coverage)
file(MAKE_DIRECTORY ${COVERAGE_DIR})

add_custom_target(
    coverage_init
    COMMAND rm -rf ${COVERAGE_DIR}/*
    COMMAND ${LCOV_EXECUTABLE} --directory ${CMAKE_BINARY_DIR} --capture --output-file ${COVERAGE_DIR}/base.info --initial
)

add_custom_target(
    coverage_generate
    COMMAND ${LCOV_EXECUTABLE} --rc lcov_branch_coverage=1 --directory ${CMAKE_BINARY_DIR} --capture --output-file ${COVERAGE_DIR}/full.info
    COMMAND ${LCOV_EXECUTABLE} --rc lcov_branch_coverage=1 --add ${COVERAGE_DIR}/base.info -add ${COVERAGE_DIR}/full.info --output-file ${COVERAGE_DIR}/report.info
    COMMAND ${LCOV_EXECUTABLE} --rc lcov_branch_coverage=1 --extract ${COVERAGE_DIR}/report.info \"${CMAKE_SOURCE_DIR}/projects/*\" --output-file ${COVERAGE_DIR}/sources.info
    COMMAND ${LCOV_EXECUTABLE} --rc lcov_branch_coverage=1 --remove ${COVERAGE_DIR}/sources.info \"${CMAKE_BINARY_DIR}/*\" --output-file ${COVERAGE_DIR}/coverage.info
    COMMAND ${LCOV_EXECUTABLE} --rc lcov_branch_coverage=1 --remove ${COVERAGE_DIR}/coverage.info \"${CMAKE_SOURCE_DIR}/**/test/*\" --output-file ${COVERAGE_DIR}/final_coverage.info
    COMMAND ${GENHTML_EXECUTABLE} ${COVERAGE_DIR}/final_coverage.info -o coverage --prefix ${CMAKE_SOURCE_DIR} --demangle-cpp
)

add_custom_target(
    coverage_serve
    COMMAND ${NPX_EXECUTABLE} -y http-server ${COVERAGE_DIR}
)

add_compile_options(-fprofile-arcs)
add_compile_options(-ftest-coverage)
add_link_options(-fprofile-arcs)
add_link_options(-lgcov)
