#pragma once
#include <mig/core/engine.hpp>
#include <mig/hands/frame.hpp>
namespace mig::hands {
struct HandObservations {
    FingerObservations fingers{};
    std::array<HandContact, 2> contacts{};
    std::array<int, 2> hand_indices{-1, -1}; // Anatomical side -> raw hand, or unavailable.
};
HandObservations hand_observations(const Frame& hands, const mig::Frame& body) noexcept;
FingerObservations finger_observations(const Frame& hands, const mig::Frame& body) noexcept;
} // namespace mig::hands
