vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO njaldea/nil
    REF 2eeef3b3514ca9ce052be31f6c50c7685e35173e
    SHA512 e99aa2e8146e491023e04df1b57cd36888a494f853b66ba2f389ba588a8d9f1a473ad086907918cd2ec2d0255388e2a13604eccb9c899f6b661163e4f35d6c8b
    HEAD_REF master
)

vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
        cli       ENABLE_FEATURE_CLI
)

if(NOT FEATURE_OPTIONS MATCHES "^.*=ON.*$")
    string(ASCII 27 Esc)
    set(Red         "${Esc}[1;31m")
    set(ColorReset  "${Esc}[m")
    message(
        "\nAvailable features:"
        "\n   -  cli"
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