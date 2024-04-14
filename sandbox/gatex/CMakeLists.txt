project(sandbox_gatex)

set(
    SOURCES
        main.cpp
        Command.cpp
)

set(
    HEADERS
        Command.hpp
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} PRIVATE sandbox_nedit_proto)
target_link_libraries(${PROJECT_NAME} PRIVATE dev)
target_link_libraries(${PROJECT_NAME} PRIVATE cli)
target_link_libraries(${PROJECT_NAME} PRIVATE service)
target_link_libraries(${PROJECT_NAME} PRIVATE gatex)
target_compile_definitions(${PROJECT_NAME} PRIVATE BOOST_ASIO_NO_DEPRECATED)
