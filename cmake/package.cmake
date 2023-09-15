include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

configure_package_config_file(
	cmake/config.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}-config.cmake
	INSTALL_DESTINATION ${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME}
	NO_SET_AND_CHECK_MACRO
)
    
write_basic_package_version_file(
	${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}-config-version.cmake
	VERSION 0.0.1
	COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}-config-version.cmake"
    DESTINATION "share/${CMAKE_PROJECT_NAME}"
)

# standardization in folder structure will be necessary
# publish folder should contain all headers that needs to be published
# <root_project>-<target>-targets.cmake will be created
function(install_for_package TARGET)
	install(DIRECTORY publish/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
	install(TARGETS ${TARGET} EXPORT ${CMAKE_PROJECT_NAME}-${TARGET}-targets)
	install(
		EXPORT ${CMAKE_PROJECT_NAME}-${TARGET}-targets
		NAMESPACE ${CMAKE_PROJECT_NAME}::
		DESTINATION ${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME}
	)
endfunction()