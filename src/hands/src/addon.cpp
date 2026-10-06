#include <cmath>
#include <mig/hands/addon.hpp>
#include <mig/hands/frame.hpp>

namespace mig::hands {

bool valid(const Hand& hand) noexcept {
    if (!std::isfinite(hand.handedness_score) || hand.handedness_score < 0 ||
        hand.handedness_score > 1) {
        return false;
    }
    if (hand.model_side != ModelSide::unknown && hand.model_side != ModelSide::left &&
        hand.model_side != ModelSide::right) {
        return false;
    }
    for (const auto p : hand.points) {
        if (!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.z) ||
            std::abs(p.x) > 16 || std::abs(p.y) > 16 || std::abs(p.z) > 16) {
            return false;
        }
    }
    return true;
}

std::string_view addon_name() noexcept {
    return "mig-hands";
}

} // namespace mig::hands
