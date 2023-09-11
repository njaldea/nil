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

include(CMakePackageConfigHelpers)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/nil-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/nil-config-version.cmake"
    DESTINATION "share/nil"
)

configure_package_config_file(cmake/config.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/nil-config.cmake
	INSTALL_DESTINATION ${CMAKE_INSTALL_DATADIR}/nil
	NO_SET_AND_CHECK_MACRO)
    
write_basic_package_version_file(
	${CMAKE_CURRENT_BINARY_DIR}/nil-config-version.cmake
	VERSION 0.0.1
	COMPATIBILITY SameMajorVersion)
