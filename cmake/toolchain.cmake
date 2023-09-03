set(CMAKE_CXX_STANDARD 20)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/out)

# only needed if there are shared libraries to be installed
set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
set(CMAKE_BUILD_WITH_INSTALL_RPATH true)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-flto)
add_compile_options(-Werror)
add_compile_options(-Wall)
add_compile_options(-Wextra)
add_compile_options(-Wpedantic)
add_compile_options(-pedantic-errors)
add_compile_options(-Wconversion)
add_compile_options(-Wsign-conversion)

# expected directory tree
#   - nil/projects/p1
#       - a.cpp
#       - b.hpp
#       - internal.hpp
#       - include/nil/p1/a.hpp
#
# this will make includes:
# #include <p1/a.hpp>           // for public/published headers
# #include <p1/b.hpp>           // for headers visible to other modules
# #include <p1/internal.hpp>    // for headers only not visible to other modules
# include_directories(${CMAKE_SOURCE_DIR}/projects)

# alternative directory tree
#   - nil/projects/p1
#       - published/nil/p1/a.hpp
#       - internal/p1/b.hpp
#       - src/a.cpp
#       - src/internal.cpp
#
# this will make includes:
# #include <nil/p1/a.hpp>   // for public/published headers
# #include <p1/b.hpp>       // for headers visible to other modules
# #include "internal.hpp"   // for headers only not visible to other modules

# include/published is to make it easier to "install" (simply by copying the directories)
# ofc, this depends on how users will consume the public library.
# for me, ideally, users need to do the following include
# #include <nil/p1/a.hpp> which is
# #include <[mono repo name]/[sub project name]/[header file path]>