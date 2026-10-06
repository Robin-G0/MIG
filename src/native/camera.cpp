#include "camera.hpp"
#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <mutex>
#include <stdexcept>
#include <wrl/implements.h>

namespace mig::native {
using Microsoft::WRL::ComPtr;
class Camera::ReaderCallback final
    : public Microsoft::WRL::RuntimeClass<
          Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>, IMFSourceReaderCallback> {
public:
    std::mutex mutex;
    std::condition_variable ready;
    bool stopped{}, received{};
    HRESULT status{};
    DWORD flags{};
    LONGLONG timestamp{};
    ComPtr<IMFSample> sample;
    HRESULT STDMETHODCALLTYPE OnReadSample(HRESULT hr, DWORD, DWORD stream_flags, LONGLONG time,
                                           IMFSample* result) override {
        {
            std::lock_guard lock(mutex);
            if (stopped) {
                return S_OK;
            }
            status = hr;
            flags = stream_flags;
            timestamp = time;
            sample = result;
            received = true;
        }
        ready.notify_all();
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE OnFlush(DWORD) override {
        ready.notify_all();
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE OnEvent(DWORD, IMFMediaEvent*) override {
        return S_OK;
    }
    void stop() {
        {
            std::lock_guard lock(mutex);
            stopped = true;
            sample.Reset();
        }
        ready.notify_all();
    }
};
namespace {
constexpr DWORD video_stream = static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM);
constexpr DWORD all_streams = static_cast<DWORD>(MF_SOURCE_READER_ALL_STREAMS);
void check(HRESULT result, const char* what) {
    if (FAILED(result)) {
        throw std::runtime_error(std::string(what) + " failed (HRESULT " + std::to_string(result) +
                                 ")");
    }
}
} // namespace
std::int64_t now_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now().time_since_epoch())
        .count();
}
Camera::Camera(unsigned index) {
    ComPtr<IMFAttributes> attributes;
    check(MFCreateAttributes(&attributes, 2), "Camera attributes");
    check(attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
                              MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID),
          "Camera type");
    IMFActivate** devices{};
    UINT32 count{};
    check(MFEnumDeviceSources(attributes.Get(), &devices, &count), "Enumerate cameras");
    struct Devices {
        IMFActivate** data;
        UINT32 count;
        ~Devices() {
            for (UINT32 i = 0; i < count; ++i) {
                data[i]->Release();
            }
            CoTaskMemFree(data);
        }
    } cleanup{devices, count};
    if (index >= count) {
        throw std::runtime_error(
            "Camera not found; close other webcam applications or change --camera");
    }
    check(devices[index]->ActivateObject(IID_PPV_ARGS(&source_)), "Open camera");
    try {
        check(MFCreateAttributes(&attributes, 3), "Reader attributes");
        check(attributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE),
              "Video conversion");
        check(attributes->SetUINT32(MF_LOW_LATENCY, TRUE), "Low latency");
        callback_ = Microsoft::WRL::Make<ReaderCallback>();
        check(attributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, callback_.Get()),
              "Async camera callback");
        check(MFCreateSourceReaderFromMediaSource(source_.Get(), attributes.Get(), &reader_),
              "Camera reader");
        check(reader_->SetStreamSelection(all_streams, FALSE), "Disable streams");
        check(reader_->SetStreamSelection(video_stream, TRUE), "Enable video");
        // Prefer a native widescreen mode rather than the driver's first 4:3
        // format. Keep capture bounded to 720p and favour real-time frame rates.
        struct Mode {
            ComPtr<IMFMediaType> type;
            double score;
        };
        std::vector<Mode> modes;
        for (DWORD i = 0;; ++i) {
            ComPtr<IMFMediaType> type;
            if (FAILED(reader_->GetNativeMediaType(video_stream, i, &type))) {
                break;
            }
            UINT32 w{}, h{};
            MFGetAttributeSize(type.Get(), MF_MT_FRAME_SIZE, &w, &h);
            UINT32 numerator{}, denominator{};
            MFGetAttributeRatio(type.Get(), MF_MT_FRAME_RATE, &numerator, &denominator);
            const double fps = denominator ? double(numerator) / denominator : 0;
            if (w >= 640 && w <= 1280 && h >= 360 && h <= 720) {
                const bool wide = double(w) / h >= 1.7;
                modes.push_back({type, (fps >= 24 ? 10000.0 : 0.0) + (wide ? 1000.0 : 0.0) +
                                           std::min(fps, 60.0) * 10 + double(w) * h / 1000000});
            }
        }
        std::stable_sort(modes.begin(), modes.end(),
                         [](const auto& a, const auto& b) { return a.score > b.score; });
        for (const auto& mode : modes) {
            if (SUCCEEDED(reader_->SetCurrentMediaType(video_stream, nullptr, mode.type.Get()))) {
                break;
            }
        }
        ComPtr<IMFMediaType> requested;
        check(MFCreateMediaType(&requested), "Video type");
        requested->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
        requested->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
        check(reader_->SetCurrentMediaType(video_stream, nullptr, requested.Get()),
              "RGB32 camera output");
        ComPtr<IMFMediaType> actual;
        check(reader_->GetCurrentMediaType(video_stream, &actual), "Video dimensions");
        UINT32 w{}, h{};
        check(MFGetAttributeSize(actual.Get(), MF_MT_FRAME_SIZE, &w, &h), "Frame size");
        if (w == 0 || h == 0 || w > 4096 || h > 4096) {
            throw std::runtime_error("Unsupported camera dimensions");
        }
        width_ = static_cast<int>(w);
        height_ = static_cast<int>(h);
        UINT32 stride{};
        if (SUCCEEDED(actual->GetUINT32(MF_MT_DEFAULT_STRIDE, &stride))) {
            stride_ = static_cast<LONG>(stride);
        } else {
            check(MFGetStrideForBitmapInfoHeader(MFVideoFormat_RGB32.Data1, width_, &stride_),
                  "Video stride");
        }
    } catch (...) {
        shutdown();
        throw;
    }
}
Camera::~Camera() {
    shutdown();
}
void Camera::shutdown() noexcept {
    if (callback_) {
        callback_->stop();
    }
    if (reader_) {
        reader_->Flush(video_stream);
    }
    if (source_) {
        source_->Shutdown();
    }
}
bool Camera::read(VideoFrame& frame) {
    return read(frame, true);
}
bool Camera::read(VideoFrame& frame, bool include_display_buffer) {
    DWORD flags{};
    LONGLONG timestamp{};
    ComPtr<IMFSample> sample;
    {
        std::lock_guard lock(callback_->mutex);
        if (callback_->stopped) {
            return false;
        }
        callback_->received = false;
    }
    check(reader_->ReadSample(video_stream, 0, nullptr, nullptr, nullptr, nullptr),
          "Request camera sample");
    {
        std::unique_lock lock(callback_->mutex);
        if (!callback_->ready.wait_for(lock, std::chrono::seconds(3),
                                       [&] { return callback_->stopped || callback_->received; })) {
            throw std::runtime_error("Camera sample timed out");
        }
        if (callback_->stopped) {
            return false;
        }
        check(callback_->status, "Camera read");
        flags = callback_->flags;
        timestamp = callback_->timestamp;
        sample = std::move(callback_->sample);
    }
    if (flags & (MF_SOURCE_READERF_ENDOFSTREAM | MF_SOURCE_READERF_ERROR)) {
        throw std::runtime_error("Camera stream ended");
    }
    if (flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED) {
        throw std::runtime_error("Camera format changed; restart and recalibrate");
    }
    if (!sample) {
        return false;
    }
    // Align the source clock once; retain sample-time deltas so a queued frame
    // is not assigned the inference/dequeue timestamp of a later frame.
    const auto sample_ms = timestamp / 10000;
    const auto received = now_ms();
    if (!clock_set_) {
        clock_offset_ = received - sample_ms;
        clock_set_ = true;
    }
    if (last_timestamp_ >= 0 && sample_ms < last_timestamp_) {
        throw std::runtime_error("Camera clock reset; restart calibration");
    }
    last_timestamp_ = sample_ms;
    clock_offset_ = std::min(clock_offset_, received - sample_ms);
    frame.capture_ms = sample_ms + clock_offset_;
    frame.width = width_;
    frame.height = height_;
    ComPtr<IMFMediaBuffer> buffer;
    check(sample->ConvertToContiguousBuffer(&buffer), "Camera buffer");
    BYTE* data{};
    DWORD size{};
    check(buffer->Lock(&data, nullptr, &size), "Camera buffer lock");
    struct Unlock {
        IMFMediaBuffer* b;
        ~Unlock() {
            b->Unlock();
        }
    } unlock{buffer.Get()};
    // Widen before abs: abs(LONG_MIN) is undefined for malformed media attributes.
    const auto stride = static_cast<std::size_t>(std::abs(static_cast<std::int64_t>(stride_)));
    if (stride < std::size_t(width_) * 4 || size < stride * height_) {
        throw std::runtime_error("Camera buffer too small");
    }
    if (include_display_buffer) {
        frame.bgrx.resize(std::size_t(width_) * height_ * 4);
    } else {
        frame.bgrx.clear();
    }
    frame.rgb.resize(std::size_t(width_) * height_ * 3);
    for (int y = 0; y < height_; ++y) {
        const auto* row = data + (stride_ >= 0 ? y : height_ - 1 - y) * stride;
        if (include_display_buffer) {
            auto* out = frame.bgrx.data() + std::size_t(y) * width_ * 4;
            std::memcpy(out, row, std::size_t(width_) * 4);
        }
        for (int x = 0; x < width_; ++x) {
            const auto offset = (std::size_t(y) * width_ + x) * 3;
            frame.rgb[offset] = row[x * 4 + 2];
            frame.rgb[offset + 1] = row[x * 4 + 1];
            frame.rgb[offset + 2] = row[x * 4];
        }
    }
    return true;
}
} // namespace mig::native
