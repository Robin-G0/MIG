#pragma once
#include "pose.hpp"
#ifndef MP_EXPORT
#define MP_EXPORT
#endif
#include <mediapipe/tasks/c/vision/pose_landmarker/pose_landmarker.h>
#ifdef MIG_NATIVE_HANDS
#include <mediapipe/tasks/c/vision/hand_landmarker/hand_landmarker.h>
#endif

namespace mig::native::detail {
Frame body_observations(const PoseLandmarkerResult& result, std::int64_t t, std::uint64_t sequence,
                        float aspect) noexcept;
#ifdef MIG_NATIVE_HANDS
hands::Frame hand_observations(const HandLandmarkerResult& result, std::int64_t t,
                               std::uint64_t sequence, float aspect) noexcept;
#endif
} // namespace mig::native::detail
