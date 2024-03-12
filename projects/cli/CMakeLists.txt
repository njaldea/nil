project(cli)

if(NOT ENABLE_FEATURE_CLI)
    return()
endif()

find_package(Boost COMPONENTS program_options REQUIRED)

set(
    HEADERS
        publish/nil/cli.hpp
        publish/nil/cli/Builder.hpp
        publish/nil/cli/Node.hpp
        publish/nil/cli/Command.hpp
        publish/nil/cli/Options.hpp
        publish/nil/cli/IOption.hpp
        publish/nil/cli/OptionInfo.hpp
        publish/nil/cli/conf/Flag.hpp
        publish/nil/cli/conf/Number.hpp
        publish/nil/cli/conf/Param.hpp
        publish/nil/cli/conf/Params.hpp
        publish/nil/cli/nodes/Help.hpp
        src/common.hpp
)

set(
    SOURCES
        src/Builder.cpp
        src/Node.cpp
        src/Options.cpp
        src/nodes/Help.cpp
)

add_library(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} PRIVATE Boost::program_options)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/publish>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

add_test_subdirectory()
install_for_package(${PROJECT_NAME})
