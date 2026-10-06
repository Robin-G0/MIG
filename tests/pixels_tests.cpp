#include "../src/native/pixels.hpp"
#include <array>
#include <iostream>

int main() {
    const std::array<std::uint8_t, 12> source{16, 128, 235, 128, 99, 99, 81, 90, 145, 240, 99, 99};
    mig::native::VideoFrame rgb, display;
    rgb.width = display.width = 2;
    rgb.height = display.height = 2;
    mig::native::convert_yuyv<false>(source, 6, rgb);
    mig::native::convert_yuyv<true>(source, 6, display);
    const std::vector<std::uint8_t> expected{0, 0, 0, 255, 255, 255, 255, 0, 0, 255, 74, 74};
    if (rgb.rgb != expected || rgb.rgb != display.rgb || !rgb.bgrx.empty()) {
        std::cerr << "RGB-only conversion must retain identical color and respect row padding.\n";
        return 1;
    }
    for (std::size_t pixel = 0; pixel < 4; ++pixel) {
        if (display.bgrx[pixel * 4] != rgb.rgb[pixel * 3 + 2] ||
            display.bgrx[pixel * 4 + 1] != rgb.rgb[pixel * 3 + 1] ||
            display.bgrx[pixel * 4 + 2] != rgb.rgb[pixel * 3] ||
            display.bgrx[pixel * 4 + 3] != 255) {
            return 1;
        }
    }
    mig::native::convert_yuyv<false>(source, 6, display);
    return display.bgrx.empty() && display.rgb == expected ? 0 : 1;
}
