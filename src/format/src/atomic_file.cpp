#include "atomic_file.hpp"
#include <atomic>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <system_error>
#include <thread>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

namespace mig::format::detail {
namespace {
#ifdef _WIN32
void replace_windows_file(const std::filesystem::path& temporary,
                          const std::filesystem::path& destination) {
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(500);
    for (;;) {
        if (MoveFileExW(temporary.c_str(), destination.c_str(),
                        MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
            return;
        }
        const auto error = GetLastError();
        const bool contention = error == ERROR_SHARING_VIOLATION || error == ERROR_LOCK_VIOLATION ||
                                error == ERROR_ACCESS_DENIED;
        if (!contention || std::chrono::steady_clock::now() >= deadline) {
            throw std::system_error(static_cast<int>(error), std::system_category(),
                                    "Cannot replace configuration");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}
#endif

class StagingFile {
public:
    explicit StagingFile(const std::filesystem::path& destination) {
        static std::atomic<std::uint64_t> serial{};
        for (int attempt = 0; attempt < 16; ++attempt) {
            const auto time = std::chrono::steady_clock::now().time_since_epoch().count();
            directory_ = destination;
            directory_ += ".tmp." + std::to_string(time) + "." + std::to_string(serial++);
            file_ = directory_ / "profile.json";
            std::error_code error;
            // Atomic reservation also works for concurrent writers in other processes.
            if (std::filesystem::create_directory(directory_, error)) {
                owned_ = true;
                return;
            }
            if (error) {
                throw std::runtime_error("Cannot reserve configuration staging directory: " +
                                         error.message());
            }
        }
        throw std::runtime_error("Cannot reserve unique configuration staging directory");
    }
    ~StagingFile() {
        if (owned_) {
            std::error_code ignored;
            std::filesystem::remove(path(), ignored);
            std::filesystem::remove(directory_,
                                    ignored); // Empty owned directory only, never recursive.
        }
    }
    StagingFile(const StagingFile&) = delete;
    StagingFile& operator=(const StagingFile&) = delete;
    const std::filesystem::path& path() const noexcept {
        return file_;
    }

private:
    std::filesystem::path directory_;
    std::filesystem::path file_;
    bool owned_{};
};
} // namespace
void replace_document(const std::filesystem::path& destination, const std::string& document) {
    StagingFile staging(destination);
    const auto temporary = staging.path();
    {
        std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
        output.write(document.data(), static_cast<std::streamsize>(document.size()));
        output.close();
        if (!output) {
            throw std::runtime_error("Cannot write configuration");
        }
    }
#ifdef _WIN32
    replace_windows_file(temporary, destination);
#else
    std::error_code error;
    std::filesystem::rename(temporary, destination, error);
    if (error) {
        throw std::runtime_error("Cannot replace configuration: " + error.message());
    }
#endif
}
} // namespace mig::format::detail
