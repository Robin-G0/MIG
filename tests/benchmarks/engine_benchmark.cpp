#include "../../src/native/pixels.hpp"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <mig/core/engine.hpp>
#include <numeric>
#include <vector>

namespace {
using Clock = std::chrono::steady_clock;
double ns(Clock::time_point start, Clock::time_point finish) {
    return std::chrono::duration<double, std::nano>(finish - start).count();
}
void report(const char* stage, std::vector<double> values, double total) {
    const auto mean = std::accumulate(values.begin(), values.end(), 0.0) / values.size();
    std::sort(values.begin(), values.end());
    std::cout << " stage=" << stage << " avg_ns=" << mean
              << " median_ns=" << values[values.size() / 2]
              << " p95_ns=" << values[values.size() * 95 / 100]
              << " p99_ns=" << values[values.size() * 99 / 100]
              << " percent_measured=" << (total ? 100 * mean / total : 100) << '\n';
}
mig::Frame frame(std::int64_t time) {
    mig::Frame result;
    result.timestamp_ms = time;
    result.sequence = static_cast<std::uint64_t>(time);
    result.aspect = 1;
    result.points[11] = {{0.65f, 0.45f}, 1};
    result.points[12] = {{0.35f, 0.45f}, 1};
    return result;
}
void measure(std::size_t motions, std::size_t points) {
    mig::Configuration profile;
    for (std::size_t i = 0; i < motions; ++i) {
        mig::Motion motion;
        motion.id = "motion_" + std::to_string(i);
        mig::Step step{"route", mig::StepMode::Ordered};
        for (std::size_t j = 0; j < points; ++j) {
            step.constraints.push_back(
                {"cell_" + std::to_string(j),
                 15,
                 {4, 5 - int(j % 4)},
                 j + 1 == points ? mig::ConstraintType::Trigger : mig::ConstraintType::Required});
        }
        motion.steps = {std::move(step)};
        profile.motions.push_back(std::move(motion));
    }
    mig::Engine engine(std::move(profile));
    for (int i = 1; i <= 25; ++i) {
        const auto pose = frame(i * 50);
        engine.update(pose, pose.timestamp_ms);
    }
    constexpr int updates = 100000;
    std::vector<double> setup(updates), recognition(updates);
    std::size_t events = 0;
    for (int i = 0; i < updates; ++i) {
        const auto start = Clock::now();
        auto pose = frame(1300 + std::int64_t(i) * 30);
        const float y = 5.5f - float(i % 4);
        pose.points[15] = {engine.grid().metric({4.5f, y}), 1};
        const auto ready = Clock::now();
        events += engine.update(pose, pose.timestamp_ms).size();
        const auto finish = Clock::now();
        setup[i] = ns(start, ready);
        recognition[i] = ns(ready, finish);
    }
    const auto total = (std::accumulate(setup.begin(), setup.end(), 0.0) +
                        std::accumulate(recognition.begin(), recognition.end(), 0.0)) /
                       updates;
    std::cout << "motions=" << motions << " points=" << points << " ns/update=" << total
              << " events=" << events << '\n';
    report("frame_setup", std::move(setup), total);
    report("engine_update", std::move(recognition), total);
}
void measure_pixels(int width, int height) {
    mig::native::VideoFrame video;
    video.width = width;
    video.height = height;
    std::vector<std::uint8_t> source(std::size_t(width) * height * 2, 128);
    mig::native::convert_yuyv<false>(source, width * 2, video);
    std::vector<double> samples(200);
    for (auto& sample : samples) {
        const auto start = Clock::now();
        mig::native::convert_yuyv<false>(source, width * 2, video);
        sample = ns(start, Clock::now());
    }
    std::cout << "pixels=" << width << 'x' << height << " checksum=" << int(video.rgb[0]) << '\n';
    report("yuyv_to_rgb", std::move(samples), 0);
}
} // namespace
int main() {
    for (const std::size_t motions : {1u, 16u, 64u}) {
        for (const std::size_t points : {2u, 64u}) {
            measure(motions, points);
        }
    }
    measure_pixels(640, 480);
    measure_pixels(1280, 720);
}
