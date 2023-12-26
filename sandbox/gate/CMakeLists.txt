project(sandbox_gate)

if(NOT ENABLE_FEATURE_GATE OR NOT ENABLE_FEATURE_CLI)
    return()
endif()

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE cli)
target_link_libraries(${PROJECT_NAME} PRIVATE gate)