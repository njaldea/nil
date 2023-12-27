project(sandbox_service)

if(NOT ENABLE_FEATURE_SERVICE)
    return()
endif()

if (NOT ENABLE_FEATURE_CLI)
    status(FATAL "ENABLE_FEATURE_CLI is required")
endif()

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE cli)
target_link_libraries(${PROJECT_NAME} PRIVATE service)

add_executable(${PROJECT_NAME}_typed typed.cpp)
target_link_libraries(${PROJECT_NAME}_typed PRIVATE cli)
target_link_libraries(${PROJECT_NAME}_typed PRIVATE service)
