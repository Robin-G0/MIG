#pragma once
#include "camera.hpp"
#include "pose.hpp"
#include <QImage>
#include <atomic>
#include <mutex>
#include <thread>
#ifdef MIG_NATIVE_HANDS
#include <mig/hands/frame.hpp>
#endif

namespace mig::linux_ui {
struct Snapshot {
    QImage image;
    Frame body;
    Grid grid;
    Grid reference_grid;
#ifdef MIG_NATIVE_HANDS
    hands::Frame hands;
#endif
    std::array<bool, 64> active{};
    std::vector<Event> events;
    std::string error;
    bool paused{};
    InputProgress progress, mirrored_progress;
    int inspected{-1};
};
// Single owning worker, one latest snapshot: slow rendering never queues images.
// Stop/join before changing the profile; old recognition cannot cross that barrier.
class Capture {
public:
    ~Capture();
    void start(Configuration config, std::string runtime, unsigned camera);
    void stop();
    bool take(Snapshot& snapshot);
    std::atomic<bool> preview_enabled{true};
    std::atomic<int> verification{-1};

private:
    void run(Configuration config, const std::string& runtime, unsigned camera);
    std::jthread worker_;
    std::atomic<bool> stopping_{};
    std::mutex mutex_;
    Snapshot latest_;
    bool pending_{};
};
} // namespace mig::linux_ui
