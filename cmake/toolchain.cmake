set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# only needed if there are shared libraries to be installed
set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
set(CMAKE_BUILD_WITH_INSTALL_RPATH true)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "[0 | OFF - 1 | ON]: Build using shared libraries?")

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-fno-rtti)
    add_compile_options(-flto)
    add_compile_options(-Wfatal-errors)
    add_compile_options(-Wshadow)
    add_compile_options(-Werror)
    add_compile_options(-Wall)
    add_compile_options(-Wextra)
    add_compile_options(-Wpedantic)
    add_compile_options(-pedantic-errors)
    add_compile_options(-Wconversion)
    add_compile_options(-Wsign-conversion)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options("/GR-")    # -fno-rtti
    add_compile_options("/GL")     # -flto
    # add_compile_options("/Wall")   # -Wall
    # add_compile_options("/WX")     # -Werror
    # add_compile_options("/wd4820") # struct padding
    # add_compile_options("/wd4464") # "../" warning for relative include
    # add_compile_options("/wd4514") # unreferenced inline function has been removed
    # [TODO] figure out how to ignore 3rd party headers
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_link_options(-fuse-ld=lld-18)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

#   TODO:
#    -  repo is currently setup using clang 18
#    -  ubuntu 22 default gcc (libstdc++) is at version 11
#    -  only one sandbox is requiring libc++ (due to stl <format>)
#    -  for all libraries exported by this repo, libc++ should not be necessary
#    -  a custom triplet for vcpkg is created for this purpose