#pragma once
#include <cstdint>
#ifdef _WIN32
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <vector>
#include <windows.h>
#include <wrl/client.h>
#else
#include <memory>
#include <vector>
#endif

namespace mig::native {
struct VideoFrame {
    int width{}, height{};
    std::int64_t capture_ms{};
    std::vector<std::uint8_t> bgrx, rgb;
};
std::int64_t now_ms();
class Camera {
public:
    explicit Camera(unsigned index);
    ~Camera();
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;
    bool read(VideoFrame& frame);
    bool read(VideoFrame& frame, bool include_display_buffer);
    void shutdown() noexcept;

private:
#ifdef _WIN32
    Microsoft::WRL::ComPtr<IMFMediaSource> source_;
    Microsoft::WRL::ComPtr<IMFSourceReader> reader_;
    int width_{}, height_{};
    LONG stride_{};
    class ReaderCallback;
    Microsoft::WRL::ComPtr<ReaderCallback> callback_;
    std::int64_t clock_offset_{}, last_timestamp_{-1};
    bool clock_set_{};
#else
    struct Impl;
    std::unique_ptr<Impl> impl_;
#endif
};
} // namespace mig::native
