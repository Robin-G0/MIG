#pragma once
#include <mig/core/engine.hpp>

namespace mig::app {
// Both applications and their editor read the same persisted switch.
constexpr bool hand_tracking_requested(const Configuration& profile) noexcept {
    return profile.track_hands;
}
} // namespace mig::app
