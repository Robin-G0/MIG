#pragma once
#include <cstdlib>
#include <filesystem>
#include <stdexcept>
#include <string>
namespace demo {
struct Options {
    std::string config, runtime;
    unsigned camera{};
    bool smoke{}, hands{}, synthetic{};
    std::filesystem::path assets;
    Options(int argc, char** argv) {
        assets = std::filesystem::absolute(argv[0]).parent_path() / "../common";
        if (!std::filesystem::exists(assets / "raised-hands.json")) {
            assets = std::filesystem::path(MIG_EXAMPLE_COMMON);
        }
        if (argc < 2 || std::string(argv[1]) == "--smoke") {
            const auto directory = std::filesystem::absolute(argv[0]).parent_path();
            const auto root = std::filesystem::path(MIG_EXAMPLE_ROOT);
            config = (directory / "../common/raised-hands.json").lexically_normal().string();
            if (!std::filesystem::exists(config)) {
                config = (root / "examples/common/raised-hands.json").string();
            }
            for (const auto& candidate :
                 {directory / "../../runtime", directory / "../runtime", root / "build/windows/bin",
                  root / "build/native-linux-deps"}) {
                if (std::filesystem::exists(candidate / "models/pose_landmarker_lite.task")) {
                    runtime = candidate.lexically_normal().string();
                    break;
                }
            }
            if (const auto installed = std::getenv("MIG_RUNTIME")) {
                runtime = installed;
            }
            smoke = synthetic = argc > 1;
            hands = true;
            if (!synthetic && runtime.empty()) {
                throw std::runtime_error(
                    "Native runtime missing: extract the examples archive or build MIG.");
            }
            return;
        }
        config = argv[1];
        for (int i = 2; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--smoke") {
                smoke = synthetic = true;
            } else if (arg == "--synthetic") {
                synthetic = true;
            } else if (arg == "--hands") {
                hands = true;
            } else if (arg == "--runtime" && i + 1 < argc) {
                runtime = argv[++i];
            } else if (arg == "--camera" && i + 1 < argc) {
                camera = unsigned(std::stoul(argv[++i]));
            } else {
                throw std::runtime_error("Unknown or incomplete option: " + arg);
            }
        }
        if (synthetic == !runtime.empty()) {
            throw std::runtime_error(
                "Choose exactly one source: --synthetic or --runtime directory");
        }
    }
};
} // namespace demo
