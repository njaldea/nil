project(sandbox_nil)

if(NOT ENABLE_FEATURE_PROTO)
    return()
endif()

find_package(Boost COMPONENTS REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE proto)

target_link_libraries(${PROJECT_NAME} PRIVATE pthread)
target_link_libraries(${PROJECT_NAME} PRIVATE Boost::boost)
