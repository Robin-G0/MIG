include(FetchContent)

function(mig_require_nlohmann_json)
    if(TARGET nlohmann_json::nlohmann_json)
        return()
    endif()

    if(EXISTS "${MIG_NATIVE_DEPS}/include/nlohmann/json.hpp")
        add_library(nlohmann_json::nlohmann_json INTERFACE IMPORTED GLOBAL)
        set_target_properties(nlohmann_json::nlohmann_json PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${MIG_NATIVE_DEPS}/include")
        return()
    endif()

    find_package(nlohmann_json 3.11 CONFIG QUIET)
    if(TARGET nlohmann_json::nlohmann_json)
        return()
    endif()

    if(MIG_DEPENDENCY_MODE STREQUAL "VCPKG" OR MIG_DEPENDENCY_MODE STREQUAL "SYSTEM")
        message(FATAL_ERROR
            "nlohmann_json was not found. Install it with vcpkg (vcpkg install), "
            "provide it through CMAKE_PREFIX_PATH, or configure with "
            "-DMIG_DEPENDENCY_MODE=FETCH.")
    endif()

    if(NOT MIG_DEPENDENCY_MODE STREQUAL "AUTO" AND NOT MIG_DEPENDENCY_MODE STREQUAL "FETCH")
        message(FATAL_ERROR "Unsupported MIG_DEPENDENCY_MODE='${MIG_DEPENDENCY_MODE}'.")
    endif()

    message(STATUS "MIG: fetching nlohmann_json v3.11.3 (not supplied by the toolchain)")
    FetchContent_Declare(nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE)
    FetchContent_MakeAvailable(nlohmann_json)
endfunction()
