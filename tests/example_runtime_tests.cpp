#include "../examples/common/runtime.hpp"
#include <chrono>
#include <fstream>

namespace {
void create_runtime(const std::filesystem::path& folder, const char* library) {
    std::filesystem::create_directories(folder / "models");
    std::ofstream(folder / "models/pose_landmarker_lite.task") << "model";
    std::ofstream(folder / library) << "library";
}
} // namespace

int main(int argc, char** argv) {
    if (argc != 2) {
        return 1;
    }
    const auto suffix = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto root = std::filesystem::path(argv[1]) / std::to_string(suffix);
    const auto executable = root / "examples/sdl2";
#ifdef _WIN32
    const auto library = "libmediapipe.dll";
    const auto other_library = "libmediapipe.so";
#else
    const auto library = "libmediapipe.so";
    const auto other_library = "libmediapipe.dll";
#endif
    create_runtime(root / "build/windows/bin", other_library);
    create_runtime(root / "build/native-linux-deps", other_library);
    if (!demo::find_runtime(executable, root).empty()) {
        return 2;
    }
    const auto preset = root / "build/release/bin";
    create_runtime(preset, library);
    if (demo::find_runtime(executable, root) != preset) {
        return 3;
    }
    const auto bundled = root / "runtime";
    create_runtime(bundled, library);
    if (demo::find_runtime(executable, root) != bundled) {
        return 4;
    }
    std::filesystem::remove(bundled / "models/pose_landmarker_lite.task");
    if (demo::find_runtime(executable, root) != preset) {
        return 5;
    }
    create_runtime(executable, library);
    return demo::find_runtime(executable, root) == executable ? 0 : 6;
}
