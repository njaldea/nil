project(sandbox_gate)

if(NOT ENABLE_FEATURE_GATE)
    return()
endif()

add_executable(${PROJECT_NAME}-basic basic.cpp)
target_link_libraries(${PROJECT_NAME}-basic PRIVATE gate)
target_link_libraries(${PROJECT_NAME}-basic PRIVATE dev)

find_package(Boost REQUIRED)
target_link_libraries(${PROJECT_NAME}-basic PRIVATE Boost::boost)
target_compile_definitions(${PROJECT_NAME}-basic PRIVATE BOOST_ASIO_NO_DEPRECATED)

add_executable(${PROJECT_NAME}-async async.cpp)
target_link_libraries(${PROJECT_NAME}-async PRIVATE gate)
target_link_libraries(${PROJECT_NAME}-async PRIVATE dev)

find_package(Boost REQUIRED)
target_link_libraries(${PROJECT_NAME}-async PRIVATE Boost::boost)
target_compile_definitions(${PROJECT_NAME}-async PRIVATE BOOST_ASIO_NO_DEPRECATED)