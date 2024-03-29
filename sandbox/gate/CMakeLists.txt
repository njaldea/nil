project(sandbox_gate)

if(NOT ENABLE_FEATURE_GATE)
    return()
endif()

find_package(Boost REQUIRED)

add_executable(${PROJECT_NAME}_basic basic.cpp)
target_link_libraries(${PROJECT_NAME}_basic PRIVATE dev)
target_link_libraries(${PROJECT_NAME}_basic PRIVATE gate)
target_link_libraries(${PROJECT_NAME}_basic PRIVATE Boost::boost)
target_compile_definitions(${PROJECT_NAME}_basic PRIVATE BOOST_ASIO_NO_DEPRECATED)

add_executable(${PROJECT_NAME}_async async.cpp)
target_link_libraries(${PROJECT_NAME}_async PRIVATE dev)
target_link_libraries(${PROJECT_NAME}_async PRIVATE gate)
target_link_libraries(${PROJECT_NAME}_async PRIVATE Boost::boost)
target_compile_definitions(${PROJECT_NAME}_async PRIVATE BOOST_ASIO_NO_DEPRECATED)

add_executable(${PROJECT_NAME}_runner runner.cpp)
target_link_libraries(${PROJECT_NAME}_runner PRIVATE dev)
target_link_libraries(${PROJECT_NAME}_runner PRIVATE gate)
target_link_libraries(${PROJECT_NAME}_runner PRIVATE Boost::boost)
target_compile_definitions(${PROJECT_NAME}_runner PRIVATE BOOST_ASIO_NO_DEPRECATED)