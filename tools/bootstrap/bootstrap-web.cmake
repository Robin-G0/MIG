# Standalone CMake script; no Python/native MediaPipe library needed for browser builds.
if(NOT DEFINED MIG_WEB_ASSETS)
    get_filename_component(MIG_WEB_ASSETS "${CMAKE_CURRENT_LIST_DIR}/../../build/native-deps" ABSOLUTE)
endif()
function(artifact relative url digest)
    set(target "${MIG_WEB_ASSETS}/${relative}")
    get_filename_component(parent "${target}" DIRECTORY)
    file(MAKE_DIRECTORY "${parent}")
    if(NOT EXISTS "${target}")
        file(DOWNLOAD "${url}" "${target}" EXPECTED_HASH "SHA256=${digest}" TLS_VERIFY ON)
    endif()
    file(SHA256 "${target}" actual)
    if(NOT actual STREQUAL digest)
        message(FATAL_ERROR "SHA256 mismatch: ${relative}")
    endif()
endfunction()
artifact("include/nlohmann/json.hpp"
    "https://raw.githubusercontent.com/nlohmann/json/v3.11.3/single_include/nlohmann/json.hpp"
    "9bea4c8066ef4a1c206b2be5a36302f8926f7fdc6087af5d20b417d0cf103ea6")
artifact("models/pose_landmarker_lite.task"
    "https://storage.googleapis.com/mediapipe-models/pose_landmarker/pose_landmarker_lite/float16/1/pose_landmarker_lite.task"
    "59929e1d1ee95287735ddd833b19cf4ac46d29bc7afddbbf6753c459690d574a")
artifact("models/hand_landmarker.task"
    "https://storage.googleapis.com/mediapipe-models/hand_landmarker/hand_landmarker/float16/1/hand_landmarker.task"
    "fbc2a30080c3c557093b5ddfc334698132eb341044ccee322ccf8bcf3607cde1")
function(license relative url)
    set(target "${MIG_WEB_ASSETS}/${relative}")
    if(NOT EXISTS "${target}")
        file(DOWNLOAD "${url}" "${target}" TLS_VERIFY ON STATUS download_status)
        list(GET download_status 0 code)
        if(NOT code EQUAL 0)
            message(FATAL_ERROR "License download failed: ${relative}: ${download_status}")
        endif()
    endif()
    file(SIZE "${target}" size)
    if(size EQUAL 0)
        message(FATAL_ERROR "License is empty: ${relative}")
    endif()
endfunction()
license("nlohmann-LICENSE" "https://raw.githubusercontent.com/nlohmann/json/v3.11.3/LICENSE.MIT")
license("MediaPipe-LICENSE" "https://raw.githubusercontent.com/google-ai-edge/mediapipe/v0.10.35/LICENSE")
artifact("Emscripten-LICENSE"
    "https://raw.githubusercontent.com/emscripten-core/emscripten/4.0.15/LICENSE"
    "620a78084fc7ca97c0b5dea9abf891f3ffcadfdbf305276f099c9c4e12fc1d86")
artifact("libcxx-LICENSE"
    "https://raw.githubusercontent.com/emscripten-core/emscripten/4.0.15/system/lib/libcxx/LICENSE.TXT"
    "539dd7aed86e8a4f12cbdd0e6c50c189c7d74847e4fecc64ce2c6ee3a01da38b")
artifact("libcxxabi-LICENSE"
    "https://raw.githubusercontent.com/emscripten-core/emscripten/4.0.15/system/lib/libcxxabi/LICENSE.TXT"
    "e2b35be49f7284a45b7baca8fc7b3ab7440e7902392b2528a457816b5bb2a15c")
artifact("compiler-rt-LICENSE"
    "https://raw.githubusercontent.com/emscripten-core/emscripten/4.0.15/system/lib/compiler-rt/LICENSE.TXT"
    "1a8f1058753f1ba890de984e48f0242a3a5c29a6a8f2ed9fd813f36985387e8d")
artifact("libunwind-LICENSE"
    "https://raw.githubusercontent.com/emscripten-core/emscripten/4.0.15/system/lib/libunwind/LICENSE.TXT"
    "b5efebcaca80879234098e52d1725e6d9eb8fb96a19fce625d39184b705f7b6d")
artifact("musl-LICENSE"
    "https://raw.githubusercontent.com/emscripten-core/emscripten/4.0.15/system/lib/libc/musl/COPYRIGHT"
    "f9bc4423732350eb0b3f7ed7e91d530298476f8fec0c6c427a1c04ade22655af")
