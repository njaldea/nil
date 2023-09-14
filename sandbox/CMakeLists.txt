if(ENABLE_FEATURE_CLI)
    add_subdirectory(cli)
endif()

if(ENABLE_FEATURE_SERVICE)
    add_subdirectory(nil)
endif()