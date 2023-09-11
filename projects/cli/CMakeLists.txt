project(cli)

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
    $<INSTALL_INTERFACE:publish>
)

install(DIRECTORY publish/ DESTINATION include)

install(TARGETS cli EXPORT cli-targets)
install(
    EXPORT cli-targets
    NAMESPACE nil::
    DESTINATION share/nil
)