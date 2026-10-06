#pragma once
#include "pose.hpp"
#include <filesystem>
#include <windows.h>
namespace mig::app {
struct App;
int run_inference_test(const std::filesystem::path& directory, bool track_hands,
                       native::PoseModel model);
int run_camera_test(const std::filesystem::path& directory, unsigned index, bool track_hands,
                    native::PoseModel model);
int run_session_test(App& app, HWND window);
int run_ui_test(App& app, HWND window);
} // namespace mig::app
