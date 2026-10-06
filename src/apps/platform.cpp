#include "app.hpp"

namespace mig::app {
std::filesystem::path executable_directory() {
    wchar_t path[32768];
    const DWORD length = GetModuleFileNameW(nullptr, path, 32768);
    if (!length || length >= 32768) {
        throw std::runtime_error("Cannot locate executable");
    }
    return std::filesystem::path(path).parent_path();
}
std::wstring wide(std::string_view text) {
    if (text.empty()) {
        return {};
    }
    const auto n = MultiByteToWideChar(CP_UTF8, 0, text.data(), int(text.size()), nullptr, 0);
    std::wstring out(n, 0);
    MultiByteToWideChar(CP_UTF8, 0, text.data(), int(text.size()), out.data(), n);
    return out;
}
std::string narrow(const std::wstring& text) {
    const auto n = WideCharToMultiByte(CP_UTF8, 0, text.data(), int(text.size()), nullptr, 0,
                                       nullptr, nullptr);
    std::string out(n, 0);
    WideCharToMultiByte(CP_UTF8, 0, text.data(), int(text.size()), out.data(), n, nullptr, nullptr);
    return out;
}
} // namespace mig::app
