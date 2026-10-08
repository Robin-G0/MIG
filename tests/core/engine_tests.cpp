#include <cmath>
#include <iostream>
#include <mig/core/engine.hpp>
#include <stdexcept>

namespace {
void check(bool success, const char* name) {
    if (!success) {
        throw std::runtime_error(name);
    }
}
mig::Frame frame(std::int64_t t) {
    mig::Frame f;
    f.timestamp_ms = t;
    f.sequence = t;
    f.aspect = 1;
    f.points[11] = {{0.65f, 0.45f}, 1};
    f.points[12] = {{0.35f, 0.45f}, 1};
    return f;
}
void calibrate(mig::Engine& e) {
    for (int i = 1; i <= 25; ++i) {
        auto f = frame(i * 50);
        e.update(f, f.timestamp_ms);
    }
}
mig::Configuration config() {
    mig::Motion input;
    input.id = "test";
    input.space = mig::CoordinateSpace::Body;
    mig::Step step{"path", mig::StepMode::Ordered};
    step.constraints = {{"start", 15, {4, 5}},
                        {"prepare", 15, {4, 4}},
                        {"fire", 15, {4, 3}, mig::ConstraintType::Trigger},
                        {"suffix", 15, {4, 2}}};
    input.steps = {step};
    return {{input}};
}
std::size_t feed(mig::Engine& e, std::int64_t t, mig::Vec2 local) {
    auto f = frame(t);
    f.points[15] = {e.grid().metric(local), 1};
    return e.update(f, t).size();
}
} // namespace
int main() {
    try {
        mig::Engine e(config());
        calibrate(e);
        check(e.grid().valid, "Calibration");
        check(std::abs(e.grid().scale - 0.06f) < 0.00001f,
              "Cell side is 20 percent of calibrated shoulder width (half previous size)");
        const auto grid_anchor = e.grid().metric({4.5f, 3.5f});
        check(mig::distance(grid_anchor, e.grid().center) < 0.00001f,
              "Smaller cells preserve the shoulder anchor");
        const auto horizontal_cell = e.grid().metric({5.5f, 3.5f});
        const auto vertical_cell = e.grid().metric({4.5f, 4.5f});
        check(std::abs(mig::distance(grid_anchor, horizontal_cell) - 0.06f) < 0.00001f &&
                  std::abs(mig::distance(grid_anchor, vertical_cell) - 0.06f) < 0.00001f,
              "Both cell dimensions are halved");
        const auto grid_roundtrip = e.grid().local(e.grid().metric({2.25f, 6.75f}));
        check(mig::distance(grid_roundtrip, {2.25f, 6.75f}) < 0.00001f,
              "Smaller-grid coordinate transforms remain inverse");
        check((mig::Grid::max_cell - mig::Grid::min_cell) * mig::Grid::cell_shoulder_ratio >= 3.6f,
              "Editor grid covers wide arm movements while keeping small cells");

        mig::Engine resizing(config());
        calibrate(resizing);
        const float baseline = resizing.grid().baseline_width;
        auto closer = frame(1300);
        closer.points[11].position.x = 0.8f;
        closer.points[12].position.x = 0.2f;
        resizing.update(closer, 1300);
        check(resizing.grid().scale > 0.06f && resizing.grid().scale < 0.12f,
              "Shoulder scale grows smoothly when approaching camera");
        for (int t = 1350; t <= 1650; t += 50) {
            closer.timestamp_ms = closer.sequence = t;
            resizing.update(closer, t);
        }
        check(std::abs(resizing.grid().scale - 0.12f) < 0.00001f &&
                  resizing.grid().baseline_width == baseline &&
                  std::abs(resizing.grid().approach - 0.5f) < 0.00001f,
              "Live scale doubles while calibration baseline remains fixed");
        auto farther = frame(1700);
        farther.points[11].position.x = 0.575f;
        farther.points[12].position.x = 0.425f;
        resizing.update(farther, 1700);
        check(resizing.grid().scale > 0.03f && resizing.grid().scale < 0.12f,
              "Shoulder scale shrinks smoothly when moving away");
        for (int t = 1750; t <= 2050; t += 50) {
            farther.timestamp_ms = farther.sequence = t;
            resizing.update(farther, t);
        }
        check(std::abs(resizing.grid().scale - 0.03f) < 0.00001f,
              "Live scale follows half-width shoulders");
        // A long gap must not interpolate from an obsolete camera distance.
        closer.timestamp_ms = closer.sequence = 2400;
        resizing.update(closer, 2400);
        check(std::abs(resizing.grid().scale - 0.12f) < 0.00001f,
              "Reacquisition immediately uses current shoulder scale");
        const auto scaled_roundtrip = resizing.grid().local(resizing.grid().metric({-4.f, 13.f}));
        check(mig::distance(scaled_roundtrip, {-4.f, 13.f}) < 0.00001f,
              "Extended negative coordinates remain reversible after rescaling");
        // Recognition uses the same scaled geometry as the editor.
        for (const mig::Vec2 point :
             {mig::Vec2{4.5f, 5.5f}, mig::Vec2{4.5f, 4.5f}, mig::Vec2{4.5f, 3.5f}}) {
            closer.timestamp_ms += 50;
            closer.sequence = closer.timestamp_ms;
            closer.points[15] = {resizing.grid().metric(point), 1};
            const auto events = resizing.update(closer, closer.timestamp_ms);
            check(events.size() == (point.y == 3.5f ? 1u : 0u),
                  "Motion recognition follows the resized grid");
        }

        mig::Engine scaling_still(config());
        calibrate(scaling_still);
        feed(scaling_still, 1300, {4.5f, 5.5f});
        auto fixed_wrist = frame(1350);
        fixed_wrist.points[15] = {scaling_still.grid().metric({4.5f, 5.5f}), 1};
        fixed_wrist.points[11].position.x = 0.8f;
        fixed_wrist.points[12].position.x = 0.2f;
        for (int t = 1350; t <= 1700; t += 50) {
            fixed_wrist.timestamp_ms = fixed_wrist.sequence = t;
            check(scaling_still.update(fixed_wrist, t).empty(),
                  "Grid rescaling cannot trigger a stationary wrist");
        }
        check(feed(e, 1300, {4.5f, 5.5f}) == 0, "Start does not trigger");
        check(feed(e, 1350, {4.5f, 4.5f}) == 0, "Preparation does not trigger");
        check(feed(e, 1400, {4.5f, 3.5f}) == 1, "Explicit trigger before suffix");
        check(feed(e, 1450, {4.5f, 3.5f}) == 0, "No repeated hit");
        check(feed(e, 1500, {4.5f, 7.5f}) == 0, "Leave start to rearm");
        check(feed(e, 1550, {4.5f, 5.5f}) == 0, "Start again");
        check(feed(e, 1600, {4.5f, 4.5f}) == 0, "Preparation again");
        check(feed(e, 1650, {4.5f, 3.5f}) == 1, "Second generic hit");
        check(feed(e, 1650, {4.5f, 3.5f}) == 0, "Duplicate timestamp");
        mig::Engine loss(config());
        calibrate(loss);
        feed(loss, 1300, {4.5f, 5.5f});
        feed(loss, 1350, {4.5f, 4.5f});
        auto missing = frame(1400);
        loss.update(missing, 1400);
        check(feed(loss, 1450, {4.5f, 3.5f}) == 0, "Loss cancels prefix");
        mig::Engine stale(config());
        calibrate(stale);
        feed(stale, 1300, {4.5f, 5.5f});
        auto f = frame(1350);
        f.points[15] = {stale.grid().metric({4.5f, 3.5f}), 1};
        check(stale.update(f, 1700).empty() && !stale.grid().valid, "Stale pose invalidated");
        mig::Engine wrong(config());
        calibrate(wrong);
        check(feed(wrong, 1300, {4.5f, 3.5f}) == 0, "Cannot enter at trigger");
        mig::Engine still(config());
        calibrate(still);
        feed(still, 1300, {4.5f, 5.5f});
        auto stationary = frame(1350);
        stationary.points[15] = {still.grid().metric({4.5f, 5.5f}), 1};
        stationary.points[11].position.y += 0.08f;
        stationary.points[12].position.y += 0.08f;
        check(still.update(stationary, 1350).empty(), "Moving grid with stationary hand");
        auto bad = config();
        bad.motions[0].steps[0].constraints[0].cell.x = mig::Grid::max_cell;
        bool rejected = false;
        try {
            mig::Engine invalid(bad);
        } catch (...) {
            rejected = true;
        }
        check(rejected, "Invalid trigger rejected");
        mig::Engine poisoned(config());
        calibrate(poisoned);
        auto future = frame(1000000);
        future.sequence = ~std::uint64_t{};
        check(poisoned.update(future, 1300).empty(), "Future input rejected");
        feed(poisoned, 1300, {4.5f, 5.5f});
        check(poisoned.grid().valid, "Future input does not poison watermark");
        feed(poisoned, 1350, {4.5f, 4.5f});
        check(feed(poisoned, 1400, {4.5f, 3.5f}) == 1, "Recovery after invalid future packet");
        auto invalid_confidence = frame(1450);
        invalid_confidence.points[11].confidence = 1.1f;
        poisoned.update(invalid_confidence, 1450);
        check(!poisoned.grid().valid, "Confidence above one rejected");

        mig::Engine high_rate(config());
        for (int i = 1; i <= 202; ++i) {
            auto sample = frame(i * 5);
            if (i > 100) {
                sample.points[11].position.x = 0.656f;
                sample.points[12].position.x = 0.344f;
            }
            high_rate.update(sample, sample.timestamp_ms);
        }
        check(high_rate.grid().valid && high_rate.grid().baseline_width > 0.31f,
              "High-rate calibration uses a real rolling median window");
        std::cout << "Engine tests passed\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
