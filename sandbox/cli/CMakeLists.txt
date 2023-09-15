project(sandbox_cli)

if(NOT ENABLE_FEATURE_CLI)
    return()
endif()

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE cli)