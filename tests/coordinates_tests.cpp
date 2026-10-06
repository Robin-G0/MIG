#include <iostream>
#include <limits>
#include <mig/core/coordinates.hpp>
int main() {
    mig::Frame frame;
    frame.points[7] = {{.4f, .2f}, 1, .1f, true, {-.1f, -.5f, .3f}, true};
    frame.points[8] = {{.6f, .2f}, .8f, .3f, true, {.1f, -.5f, .5f}, true};
    const auto head = mig::body_coordinate(frame, 33);
    const auto world = mig::body_coordinate(frame, 33, mig::CoordinateSystem::WorldHeightUp);
    if (!head || std::abs(head->x - .5f) > .0001f || std::abs(head->z - .2f) > .0001f || !world ||
        std::abs(world->y - .5f) > .0001f || std::abs(world->z - .4f) > .0001f ||
        mig::body_coordinates(frame).data() != frame.points.data()) {
        return 1;
    }
    if (mig::body_coordinate(frame, 34) || mig::body_coordinate(frame, -1) ||
        mig::body_coordinate(frame, 0)) {
        return 2;
    }
    frame.points[8].depth_valid = false;
    if (mig::body_coordinate(frame, 33)) {
        return 3;
    }
    frame.points[8].confidence = 0;
    frame.points[0] = frame.points[7];
    if (!mig::body_coordinate(frame, 33)) {
        return 4;
    }
    frame.points[0].depth = std::numeric_limits<float>::quiet_NaN();
    if (mig::body_coordinate(frame, 0)) {
        return 5;
    }
    // Every newly added edge cell is both valid configuration and recognizable.
    for (const auto cell :
         {mig::Cell{-9, -9}, mig::Cell{17, -9}, mig::Cell{-9, 17}, mig::Cell{17, 17}}) {
        mig::Motion input;
        input.id = input.name = input.action = "edge";
        input.steps = {{"step",
                        mig::StepMode::Visited,
                        {{"trigger", 15, cell, mig::ConstraintType::Trigger}}}};
        mig::Engine engine(mig::Configuration{{input}});
        mig::Grid basis;
        basis.center = {.5f, .45f};
        basis.scale = .02f;
        unsigned accepted = 0;
        for (int t = 20; t <= 1200; t += 20) {
            mig::Frame sample;
            sample.timestamp_ms = t;
            sample.sequence = t / 20;
            sample.aspect = 1;
            sample.points[11] = {{.55f, .45f}, 1};
            sample.points[12] = {{.45f, .45f}, 1};
            sample.points[15] = {basis.metric({cell.x + .5f, cell.y + .5f}), 1};
            accepted += unsigned(engine.update(sample, t).size());
        }
        if (accepted != 1) {
            return 6;
        }
    }
    std::cout << "Borrowed XYZ view, world/height axes, derived head and missing data passed\n";
}
