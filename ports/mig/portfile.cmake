if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/source.cmake")
    message(FATAL_ERROR "Generate the pinned release overlay with tools/package-source.py first.")
endif()
include("${CMAKE_CURRENT_LIST_DIR}/source.cmake")
vcpkg_download_distfile(ARCHIVE URLS "${MIG_SOURCE_URL}"
    FILENAME "mig-${VERSION}-source.tar.gz" SHA512 "${MIG_SOURCE_SHA512}")
vcpkg_extract_source_archive(SOURCE_PATH ARCHIVE "${ARCHIVE}")
vcpkg_check_features(OUT_FEATURE_OPTIONS OPTIONS
    FEATURES format MIG_BUILD_FORMAT hands MIG_BUILD_HANDS c-api MIG_BUILD_C_API)
if("c-api" IN_LIST FEATURES)
    vcpkg_check_linkage(ONLY_DYNAMIC_LIBRARY)
endif()
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}" OPTIONS ${OPTIONS}
    -DMIG_BUILD_CONFIGURATOR=OFF -DMIG_BUILD_CONTROLLER=OFF
    -DMIG_BUILD_FACE=OFF -DMIG_BUILD_TESTS=OFF -DMIG_BUILD_NATIVE_RUNTIME=OFF
    -DMIG_DEPENDENCY_MODE=SYSTEM
    "-DMIG_JSON_LICENSE=${CURRENT_INSTALLED_DIR}/share/nlohmann-json/copyright")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME MIG CONFIG_PATH lib/cmake/MIG)
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include" "${CURRENT_PACKAGES_DIR}/debug/share")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
