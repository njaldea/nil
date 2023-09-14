cmake_minimum_required(VERSION 3.27.1)

project(nil CXX)

set(ENABLE_TEST             OFF CACHE BOOL "[0 | OFF - 1 | ON]: build tests?")
set(ENABLE_SANDBOX          OFF CACHE BOOL "[0 | OFF - 1 | ON]: build sandbox?")

set(ENABLE_FEATURE_CLI      OFF CACHE BOOL "[0 | OFF - 1 | ON]: build cli?")
set(ENABLE_FEATURE_PROTO    OFF CACHE BOOL "[0 | OFF - 1 | ON]: build proto?")
set(ENABLE_FEATURE_SERVICE  OFF CACHE BOOL "[0 | OFF - 1 | ON]: build service?")

include(cmake/package.cmake)
include(cmake/toolchain.cmake)
include(cmake/quality.cmake)
include(cmake/test.cmake)

add_subdirectory(projects)

if(ENABLE_SANDBOX)
    add_subdirectory(sandbox)
endif()
