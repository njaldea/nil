project(sandbox_nedit_service)

if(NOT ENABLE_FEATURE_CLI OR NOT ENABLE_FEATURE_SERVICE OR NOT ENABLE_FEATURE_GATE)
    return()
endif()

set(
    SOURCES
        main.cpp
        Command.cpp
        Service.cpp
        Info.cpp
)

set(
    HEADERS
        Command.hpp
        Service.hpp
        Info.hpp
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} PRIVATE sandbox_nedit_proto)
target_link_libraries(${PROJECT_NAME} PRIVATE dev)
target_link_libraries(${PROJECT_NAME} PRIVATE cli)
target_link_libraries(${PROJECT_NAME} PRIVATE service)
target_link_libraries(${PROJECT_NAME} PRIVATE utils)
target_link_libraries(${PROJECT_NAME} PRIVATE gate)
target_compile_definitions(${PROJECT_NAME} PRIVATE BOOST_ASIO_NO_DEPRECATED)
