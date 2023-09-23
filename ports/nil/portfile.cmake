vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO njaldea/nil
    REF 35a953dc99afbcb55db043c1368281155faddb44
    SHA512 74c981740a3b88499dd4a416d8b4356097d0f67165e5c3957a90fa54828bfcd5f2f73b9e851f096e8ea133548521d35657d8816f1122a36e4e549ce90219c0ef
    HEAD_REF master
)

vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
        cli           ENABLE_FEATURE_CLI
        service       ENABLE_FEATURE_SERVICE
)

if(NOT FEATURE_OPTIONS MATCHES "^.*=ON.*$")
    string(ASCII 27 Esc)
    set(Red         "${Esc}[1;31m")
    set(ColorReset  "${Esc}[m")
    message(
        "\nAvailable features:"
        "\n   -  cli"
        "\n   -  service"
        "\n"
        "\n  ${Red}No feature provided. Terminating...${ColorReset}"
        "\n"
    )
    message(FATAL_ERROR "${Red}Specify at least one feature to enable${ColorReset}\n")
endif()

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        ${FEATURE_OPTIONS}
)
vcpkg_cmake_install()
vcpkg_cmake_config_fixup()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

file(
    INSTALL "${SOURCE_PATH}/LICENSE"
    DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
    RENAME copyright
)