cmake_minimum_required(VERSION 3.27.1)

project(nil CXX)

set(ENABLE_FEATURE_CLI      OFF CACHE BOOL "[0 | OFF - 1 | ON]: build cli?")
set(ENABLE_FEATURE_PULSE    OFF CACHE BOOL "[0 | OFF - 1 | ON]: build pulse?")
set(ENABLE_FEATURE_UTILS    OFF CACHE BOOL "[0 | OFF - 1 | ON]: build utils?")
set(ENABLE_FEATURE_SERVICE  OFF CACHE BOOL "[0 | OFF - 1 | ON]: build service?")
set(ENABLE_FEATURE_GATE     OFF CACHE BOOL "[0 | OFF - 1 | ON]: build gate?")

include(cmake/package.cmake)
include(cmake/toolchain.cmake)
include(cmake/quality.cmake)
include(cmake/test.cmake)
include(cmake/coverage.cmake)

add_subdirectory(projects)
add_subdirectory(sandbox)
