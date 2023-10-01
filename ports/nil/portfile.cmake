vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO njaldea/nil
    REF 7bdd6afb651a0817540335754e89215434251c4e
    SHA512 be642912083f89ed634c5cde2d500c2cc8f97c6f7e32d9ec2bb96e0546928639912e6b5e41f65c555943da5e38d19d1a6700d0a9ed995176be831250a70c5dfb
    HEAD_REF master
)

vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
        cli           ENABLE_FEATURE_CLI
        service       ENABLE_FEATURE_SERVICE
        pulse         ENABLE_FEATURE_PULSE
)

if(NOT FEATURE_OPTIONS MATCHES "^.*=ON.*$")
    string(ASCII 27 Esc)
    set(Red         "${Esc}[1;31m")
    set(ColorReset  "${Esc}[m")
    message(
        "\nAvailable features:"
        "\n   -  cli"
        "\n   -  service"
        "\n   -  pulse"
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