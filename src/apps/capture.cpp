#include "app.hpp"
#include "frame_pool.hpp"

namespace mig::app {
void App::capture_loop() {
    const auto com_status = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    try {
        if (FAILED(com_status)) {
            throw std::runtime_error("Cannot initialise capture-thread COM");
        }
        // Reuse image capacity; never rewrite a slot owned by inference or the UI.
        FramePool frame_pool;
        while (running) {
            auto frame = frame_pool.acquire();
            if (!frame) {
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
                continue;
            }
            if (camera->read(*frame, preview_enabled.load())) {
                {
                    std::lock_guard lock(capture_mutex);
                    latest = std::move(frame);
                    ++latest_sequence;
                }
                capture_ready.notify_one();
            }
        }
    } catch (const std::exception& e) {
        if (running) {
            message(e.what());
        }
        wake_and_stop_workers();
    }
    if (SUCCEEDED(com_status)) {
        CoUninitialize();
    }
}
} // namespace mig::app
