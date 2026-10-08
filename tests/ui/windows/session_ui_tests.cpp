#include "app.hpp"
#include "diagnostics.hpp"
#include <fstream>

namespace mig::app {
int run_session_test(App& app, HWND window) {
    for (int cycle = 0; cycle < 3; ++cycle) {
        app.start();
        const auto deadline = now_ms() + 10000;
        bool success = false;
        while (now_ms() < deadline && app.running) {
            MSG message{};
            while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
                TranslateMessage(&message);
                DispatchMessageW(&message);
            }
            {
                std::lock_guard lock(app.state_mutex);
                success = app.snapshot && app.snapshot->pose.sequence >= 5;
            }
            if (success) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        std::string status;
        {
            std::lock_guard lock(app.state_mutex);
            status = app.status;
        }
        app.stop();
        if (!success) {
            throw std::runtime_error("Native threaded session test failed: " + status);
        }
        std::cout << "Native camera/inference/UI session start-stop cycle " << cycle + 1
                  << " passed\n";
    }
    SendMessageW(window, WM_CLOSE, 0, 0);
    application = nullptr;
    return 0;
}
} // namespace mig::app
