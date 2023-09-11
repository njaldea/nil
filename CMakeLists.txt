cmake_minimum_required(VERSION 3.27.1)

project(mono CXX)

include(cmake/package.cmake)
include(cmake/toolchain.cmake)
include(cmake/format.cmake)
include(cmake/functions.cmake)

if(ENABLE_TEST)
    include(CTest)
    find_package(GTest CONFIG REQUIRED)
endif()

if(ENABLE_FEATURE_PROTO)
    find_package(Protobuf CONFIG REQUIRED)
endif()

find_package(Boost COMPONENTS program_options REQUIRED)

add_subdirectory(projects)

if(ENABLE_SANDBOX)
    add_subdirectory(sandbox)
endif()
