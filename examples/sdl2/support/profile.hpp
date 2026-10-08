#pragma once
#include "options.hpp"
#include <functional>
#include <memory>
#include <mig/core/engine.hpp>
#include <mig/format/configuration.hpp>
#ifdef MIG_PROFILE_EXAMPLE
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <commdlg.h>
#else
#include <QApplication>
#include <QFileDialog>
#endif
#endif

namespace demo {
#ifdef MIG_PROFILE_EXAMPLE
inline constexpr bool profile_mode = true;
#else
inline constexpr bool profile_mode = false;
#endif

inline mig::Configuration initial_configuration(const Options& options) {
    auto configuration = mig::load_configuration(options.config);
    if (profile_mode && !options.smoke) {
        configuration.motions.clear();
        configuration.track_hands = false;
    }
    return configuration;
}

inline std::filesystem::path utf8_path(std::string_view text) {
    return std::u8string(text.begin(), text.end());
}

class ProfilePicker {
#if defined(MIG_PROFILE_EXAMPLE) && !defined(_WIN32)
    std::unique_ptr<QApplication> application_;
    int* argc_{};
    char** argv_{};
#endif

public:
    ProfilePicker([[maybe_unused]] int& argc, [[maybe_unused]] char** argv) {
#if defined(MIG_PROFILE_EXAMPLE) && !defined(_WIN32)
        argc_ = &argc;
        argv_ = argv;
#endif
    }
    std::filesystem::path choose() {
#ifdef MIG_PROFILE_EXAMPLE
#ifdef _WIN32
        wchar_t path[32768]{};
        OPENFILENAMEW dialog{};
        dialog.lStructSize = sizeof(dialog);
        dialog.lpstrFilter = L"MIG profiles\0*.json\0\0";
        dialog.lpstrFile = path;
        dialog.nMaxFile = 32768;
        dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
        if (GetOpenFileNameW(&dialog)) {
            return std::filesystem::path(path);
        }
#else
        if (!application_) {
            application_ = std::make_unique<QApplication>(*argc_, argv_);
            application_->setQuitOnLastWindowClosed(false);
        }
        return utf8_path(
            QFileDialog::getOpenFileName(nullptr, "Import MIG profile", {}, "MIG profiles (*.json)")
                .toStdString());
#endif
#endif
        return {};
    }
};

inline void import_profile(mig::Engine& engine, const std::filesystem::path& path,
                           std::string& status,
                           const std::function<void(bool)>& enable_hands = {}) {
    if (path.empty()) {
        return;
    }
    try {
        mig::Engine replacement(mig::load_configuration(path));
        if (enable_hands) {
            enable_hands(replacement.configuration().track_hands);
        }
        engine = std::move(replacement);
        const auto name = path.filename().u8string();
        status.assign(name.begin(), name.end());
        status += ": recalibrating";
    } catch (const std::exception& error) {
        status = error.what();
    }
}

inline std::filesystem::path font_path(const Options& options) {
    return options.assets / "DejaVuSans.ttf";
}
} // namespace demo
