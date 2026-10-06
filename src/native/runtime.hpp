#pragma once
namespace mig::native {
// Initialize the platform capture subsystem on the camera's owning thread.
// Windows balances COM/Media Foundation; Linux needs no extra initialization.
class CaptureRuntime {
public:
    CaptureRuntime();
    ~CaptureRuntime();
    CaptureRuntime(const CaptureRuntime&) = delete;
    CaptureRuntime& operator=(const CaptureRuntime&) = delete;

private:
    bool com_owned_{};
};
} // namespace mig::native
