project(utils)

if (NOT ENABLE_FEATURE_UTILS)
    return()
endif()

set(
    HEADERS
        publish/nil/utils/traits/callable.hpp
        publish/nil/utils/traits/type.hpp
)

add_library(${PROJECT_NAME} INTERFACE ${HEADERS})

target_include_directories(
    ${PROJECT_NAME}
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/publish>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # add_compile_options(-fconcepts)
endif()

# [TODO] install only when consumed
install_for_package(${PROJECT_NAME})
