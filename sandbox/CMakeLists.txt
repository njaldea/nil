# disable sandbox for coverage to avoid
# alienation of template instantiations
# that are not really used in testing
if(ENABLE_COVERAGE)
    return()
endif()

set(ENABLE_SANDBOX  OFF CACHE BOOL "[0 | OFF - 1 | ON]: build sandbox?")
if(NOT ENABLE_SANDBOX)
    return()
endif()

add_subdirectory(dev)
add_subdirectory(pulse)
add_subdirectory(gatex)
add_subdirectory(nedit)
add_subdirectory(xit)