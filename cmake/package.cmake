include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/nil-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/nil-config-version.cmake"
    DESTINATION "share/nil"
)

configure_package_config_file(cmake/config.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/nil-config.cmake
	INSTALL_DESTINATION ${CMAKE_INSTALL_DATADIR}/nil
	NO_SET_AND_CHECK_MACRO)
    
write_basic_package_version_file(
	${CMAKE_CURRENT_BINARY_DIR}/nil-config-version.cmake
	VERSION 0.0.1
	COMPATIBILITY SameMajorVersion)