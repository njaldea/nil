project(pulse)

if(NOT ENABLE_FEATURE_PULSE)
    return()
endif()

set(
    HEADERS
        publish/nil/pulse.hpp
        publish/nil/pulse/Data.hpp
)

add_library(${PROJECT_NAME} INTERFACE ${HEADERS})
target_include_directories(
    ${PROJECT_NAME}
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/publish>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# add_test_subdirectory()
install_for_package(${PROJECT_NAME})
