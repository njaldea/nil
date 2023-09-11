cmake_minimum_required(VERSION 3.27.1)

project(mono CXX)

include(cmake/toolchain.cmake)
include(cmake/format.cmake)
include(cmake/functions.cmake)
include(CTest)

find_package(Protobuf CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(Boost COMPONENTS program_options REQUIRED)

add_subdirectory(projects)
add_subdirectory(sandbox)
