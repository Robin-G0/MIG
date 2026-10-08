#pragma once
#include <cstdlib>
#include <filesystem>
#include <stdexcept>

namespace demo {
inline bool has_runtime(const std::filesystem::path& directory) {
#ifdef _WIN32
    const auto library = "libmediapipe.dll";
#else
    const auto library = "libmediapipe.so";
#endif
    return std::filesystem::is_regular_file(directory / library) &&
           std::filesystem::is_regular_file(directory / "models/pose_landmarker_lite.task");
}

inline std::filesystem::path find_runtime(const std::filesystem::path& directory,
                                          const std::filesystem::path& root) {
    if (const auto installed = std::getenv("MIG_RUNTIME"); installed && *installed) {
        if (!has_runtime(installed)) {
            throw std::runtime_error("MIG_RUNTIME must contain the native library and pose model");
        }
        return installed;
    }
    for (const auto& candidate :
         {directory / "runtime", directory, directory / "../../runtime", directory / "../runtime", root / "runtime",
          root / "build/windows/bin", root / "build/release/bin", root / "build/debug/bin",
          root / "build/native-linux-deps", root / "distribution/windows",
          root / "distribution/linux"}) {
        if (has_runtime(candidate)) {
            return candidate.lexically_normal();
        }
    }
    return {};
}
} // namespace demo
