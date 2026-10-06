file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/../VERSION" MIG_VERSION LIMIT_COUNT 1)
set(MIG_VERSION_SOURCE "VERSION development fallback")
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${CMAKE_CURRENT_LIST_DIR}/../.git")
    get_filename_component(version_root "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
    execute_process(COMMAND "${GIT_EXECUTABLE}" -c "safe.directory=${version_root}"
        describe --tags --exact-match HEAD
        WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/.."
        RESULT_VARIABLE tag_result OUTPUT_VARIABLE exact_tag
        OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    if(tag_result EQUAL 0 AND exact_tag MATCHES "^v([0-9]+\\.[0-9]+\\.[0-9]+)$")
        set(MIG_VERSION "${CMAKE_MATCH_1}")
        set(MIG_VERSION_SOURCE "Git tag ${exact_tag}")
    endif()
endif()
if(NOT MIG_VERSION MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
    message(FATAL_ERROR "Expected X.Y.Z in VERSION or an exact vX.Y.Z Git tag")
endif()
message(STATUS "MIG ${MIG_VERSION} (${MIG_VERSION_SOURCE})")
