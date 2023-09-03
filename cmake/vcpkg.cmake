message(STATUS VCPKG = [$ENV{VCPKG}])

# set(VCPKG_TARGET_TRIPLET wasm32-emscripten)
# to prevent deletion of packages when build folder is purged
set(VCPKG_INSTALLED_DIR ${CMAKE_SOURCE_DIR}/.vcpkg_install)

include($ENV{VCPKG}/scripts/buildsystems/vcpkg.cmake)