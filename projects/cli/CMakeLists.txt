project(cli)

find_package(Boost COMPONENTS program_options REQUIRED)

set(
    HEADERS
        publish/nil/cli.hpp
        publish/nil/cli/conf.hpp
        publish/nil/cli/Builder.hpp
        publish/nil/cli/Node.hpp
        publish/nil/cli/Options.hpp
        publish/nil/cli/types.hpp
        src/common.hpp
)

set(
    SOURCES
        src/Builder.cpp
        src/Node.cpp
        src/Options.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} PRIVATE Boost::program_options)
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/publish>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

install(DIRECTORY publish/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(TARGETS cli EXPORT nil-cli-targets)
install(
    EXPORT nil-cli-targets
    NAMESPACE nil::
    DESTINATION ${CMAKE_INSTALL_DATADIR}/nil
)

if(ENABLE_TEST)
    add_subdirectory(test)
endif()