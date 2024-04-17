project(sandbox_service)

if(NOT ENABLE_FEATURE_SERVICE)
    return()
endif()

find_package(nil COMPONENTS clix CONFIG REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE nil::clix)
target_link_libraries(${PROJECT_NAME} PRIVATE service)
