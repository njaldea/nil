set(CMAKE_CXX_STANDARD 20)

set(ENABLE_TEST OFF CACHE BOOL "[0 | OFF - 1 | ON]: build tests?")
set(ENABLE_SANDBOX OFF CACHE BOOL "[0 | OFF - 1 | ON]: build sandbox?")

set(ENABLE_FEATURE_CLI OFF CACHE BOOL "[0 | OFF - 1 | ON]: build cli?")
set(ENABLE_FEATURE_PROTO OFF CACHE BOOL "[0 | OFF - 1 | ON]: build proto?")
set(ENABLE_FEATURE_SERVICE OFF CACHE BOOL "[0 | OFF - 1 | ON]: build service?")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
else()
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build.")
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/out" CACHE PATH "install path" FORCE)
else()
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/out" CACHE PATH "install path")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# only needed if there are shared libraries to be installed
set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
set(CMAKE_BUILD_WITH_INSTALL_RPATH true)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-fno-rtti)
add_compile_options(-flto)
add_compile_options(-Werror)
add_compile_options(-Wall)
add_compile_options(-Wextra)
add_compile_options(-Wpedantic)
add_compile_options(-pedantic-errors)
add_compile_options(-Wconversion)
add_compile_options(-Wsign-conversion)
