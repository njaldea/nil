project(sandbox_service)

if(NOT ENABLE_FEATURE_SERVICE OR NOT ENABLE_FEATURE_CLI)
    return()
endif()

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE cli)
target_link_libraries(${PROJECT_NAME} PRIVATE service)

add_executable(${PROJECT_NAME}_typed typed.cpp)
target_link_libraries(${PROJECT_NAME}_typed PRIVATE cli)
target_link_libraries(${PROJECT_NAME}_typed PRIVATE service)
