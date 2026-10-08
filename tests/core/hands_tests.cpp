#include <limits>
#include <mig/hands/coordinates.hpp>
#include <mig/hands/frame.hpp>

int main() {
    using namespace mig::hands;
    static_assert(std::size_t(Landmark::thumb_tip) == 4);
    static_assert(std::size_t(Landmark::index_tip) == 8);
    static_assert(std::size_t(Landmark::pinky_tip) == 20);
    Frame frame{};
    if (frame.count != 0) {
        return 1;
    }
    Hand hand{};
    hand.handedness_score = 0.9f;
    hand.points[8] = {.2f, .3f, -.1f};
    if (!hand_coordinate(hand, Landmark::index_tip) ||
        hand_coordinate(hand, Landmark::index_tip, mig::CoordinateSystem::World) ||
        hand_coordinates(hand).data() != hand.points.data()) {
        return 10;
    }
    hand.world_valid = true;
    hand.world_points[8] = {.1f, -.2f, .3f};
    const auto world =
        hand_coordinate(hand, Landmark::index_tip, mig::CoordinateSystem::WorldHeightUp);
    if (!world || world->y != .2f || world->z != .3f) {
        return 11;
    }
    if (!valid(hand)) {
        return 2;
    }
    hand.points[8].x = std::numeric_limits<float>::quiet_NaN();
    if (valid(hand)) {
        return 3;
    }
    hand.points[8] = {};
    hand.points[20].z = std::numeric_limits<float>::infinity();
    if (valid(hand)) {
        return 4;
    }
    hand.points[20] = {};
    hand.handedness_score = 1.1f;
    if (valid(hand)) {
        return 5;
    }
    hand.handedness_score = 0.9f;
    hand.points[0].x = 17;
    if (valid(hand)) {
        return 6;
    }
    hand.points[0] = {};
    hand.model_side = static_cast<ModelSide>(99);
    if (valid(hand)) {
        return 7;
    }
}
