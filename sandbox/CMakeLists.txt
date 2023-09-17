# disable sandbox for coverage to avoid
# alienation of template instantiations
# that are not really used in testing
if(ENABLE_COVERAGE)
    return()
endif()

add_subdirectory(cli)
add_subdirectory(service)
