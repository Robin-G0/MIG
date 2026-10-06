#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

namespace mig::hands {
enum class Landmark : std::size_t {
    wrist,
    thumb_cmc,
    thumb_mcp,
    thumb_ip,
    thumb_tip,
    index_mcp,
    index_pip,
    index_dip,
    index_tip,
    middle_mcp,
    middle_pip,
    middle_dip,
    middle_tip,
    ring_mcp,
    ring_pip,
    ring_dip,
    ring_tip,
    pinky_mcp,
    pinky_pip,
    pinky_dip,
    pinky_tip
};
struct Position {
    float x{}, y{}, z{};
};
// Raw classifier label, not a calibrated anatomical assignment or tracking ID.
enum class ModelSide { unknown, left, right };
struct Hand {
    std::array<Position, 21>
        points{}; // Unmirrored normalized image x/y; z relative to wrist, not metres.
    ModelSide model_side{ModelSide::unknown};
    float handedness_score{};                // Classification score, NOT per-landmark confidence.
    std::array<Position, 21> world_points{}; // Metres relative to this hand's centre.
    bool world_valid{};
};
struct Frame {
    std::int64_t timestamp_ms{};
    std::uint64_t sequence{};
    float aspect{1.f};
    std::array<Hand, 2> hands{};
    std::size_t count{}; // Only [0,count) is valid; no persistent identity implied.
};
[[nodiscard]] bool valid(const Hand& hand) noexcept;
} // namespace mig::hands
