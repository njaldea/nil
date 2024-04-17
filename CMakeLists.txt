cmake_minimum_required(VERSION 3.27.1)

project(nil CXX)

set(ENABLE_FEATURE_PULSE    OFF CACHE BOOL "[0 | OFF - 1 | ON]: build pulse?")

include(cmake/package.cmake)
include(cmake/toolchain.cmake)
include(cmake/quality.cmake)
include(cmake/test.cmake)
include(cmake/coverage.cmake)

add_subdirectory(projects)
add_subdirectory(sandbox)
