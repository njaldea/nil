project(cli)

set(
    HEADERS
        public/cli/conf.hpp
        public/cli/Builder.hpp
        public/cli/Node.hpp
        public/cli/Options.hpp
        public/cli/types.hpp
        src/common.hpp
)

set(
    SOURCES
        src/Builder.cpp
        src/Node.cpp
        src/Options.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} PRIVATE Boost::program_options)
target_include_directories(${PROJECT_NAME} PUBLIC public)
