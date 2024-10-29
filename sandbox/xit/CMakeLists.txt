project(sandbox_xit)

find_package(nil-xit CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE nil::xit)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)