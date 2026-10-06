#pragma once
#include "camera.hpp"
#include <algorithm>
#include <span>

namespace mig::native {
inline std::uint8_t color_byte(int value) {
    return std::uint8_t(std::clamp(value, 0, 255));
}
template <bool Display>
void convert_yuyv(std::span<const std::uint8_t> source, std::size_t stride, VideoFrame& frame) {
    frame.rgb.resize(std::size_t(frame.width) * frame.height * 3);
    if constexpr (Display) {
        frame.bgrx.resize(std::size_t(frame.width) * frame.height * 4);
    } else {
        frame.bgrx.clear();
    }
    for (int y = 0; y < frame.height; ++y) {
        const auto* row = source.data() + y * stride;
        for (int x = 0; x < frame.width; x += 2) {
            const int u = row[x * 2 + 1] - 128, v = row[x * 2 + 3] - 128;
            for (int pixel = 0; pixel < 2; ++pixel) {
                const int luminance = std::max(0, int(row[x * 2 + pixel * 2]) - 16) * 298;
                const auto red = color_byte((luminance + 409 * v + 128) >> 8);
                const auto green = color_byte((luminance - 100 * u - 208 * v + 128) >> 8);
                const auto blue = color_byte((luminance + 516 * u + 128) >> 8);
                const auto offset = std::size_t(y) * frame.width + x + pixel;
                auto* rgb = frame.rgb.data() + offset * 3;
                rgb[0] = red;
                rgb[1] = green;
                rgb[2] = blue;
                if constexpr (Display) {
                    auto* bgr = frame.bgrx.data() + offset * 4;
                    bgr[0] = blue;
                    bgr[1] = green;
                    bgr[2] = red;
                    bgr[3] = 255;
                }
            }
        }
    }
}
} // namespace mig::native
