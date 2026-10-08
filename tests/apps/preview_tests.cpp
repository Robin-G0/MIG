#include "../../src/apps/preview.hpp"
#include <cmath>
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#endif

int main() {
    const mig::ui::PreviewTransform preview{20, 100, 640, 480, 4.f / 3.f};
    const auto close = [](float a, float b) { return std::abs(a - b) < 0.0001f; };
    const auto left = preview.screen({0, 0});
    const auto right = preview.screen({preview.aspect, 1});
    if (!close(left.x, 659) || !close(left.y, 100) || !close(right.x, 20) || !close(right.y, 579)) {
        return 1;
    }
    for (const mig::Vec2 metric : {mig::Vec2{0, 0}, mig::Vec2{preview.aspect, 1},
                                   mig::Vec2{0.3f, 0.7f}, mig::Vec2{-0.2f, 1.2f}}) {
        const auto roundtrip = preview.metric(preview.screen(metric));
        if (!close(roundtrip.x, metric.x) || !close(roundtrip.y, metric.y)) {
            return 2;
        }
    }
    // Clicking the displayed marker must recover the same unmirrored grid point.
    const auto click = preview.metric({20, 100});
    if (!close(click.x, preview.aspect) || !close(click.y, 0)) {
        return 3;
    }
    mig::ui::GridView view;
    view.live = true;
    view.preview = preview;
    view.grid.center = {.65f, .45f};
    view.grid.axis = {.8f, .6f};
    view.grid.scale = .06f;
    const auto origin = view.screen({4.5f, 3.5f});
    const auto cell = view.screen({5.5f, 3.5f});
    const float first_size = mig::distance(origin, cell);
    view.grid.scale *= 2;
    const float second_size = mig::distance(view.screen({4.5f, 3.5f}), view.screen({5.5f, 3.5f}));
    if (!close(second_size, first_size * 2)) {
        return 6;
    }
    for (const mig::Vec2 point :
         {mig::Vec2{-4, 12}, mig::Vec2{4.5f, 3.5f}, mig::Vec2{6.5f, 5.5f}}) {
        const auto roundtrip = view.local(view.screen(point));
        if (!close(roundtrip.x, point.x) || !close(roundtrip.y, point.y)) {
            return 7;
        }
    }
    view.live = false;
    view.left = 20;
    view.top = 155;
    view.side = 540;
    if (view.screen({6, 3}).x >= view.screen({2, 3}).x) {
        return 8;
    }
    for (const mig::Vec2 point : {mig::Vec2{-9, -9}, mig::Vec2{18, 18}, mig::Vec2{6.5f, 3.5f}}) {
        const auto roundtrip = view.local(view.screen(point));
        if (!close(roundtrip.x, point.x) || !close(roundtrip.y, point.y)) {
            return 9;
        }
    }
    const auto edge = view.screen({float(mig::Grid::max_cell), float(mig::Grid::min_cell)});
    if (!close(edge.x, view.left) || !close(edge.y, view.top)) {
        return 10;
    }
#ifdef _WIN32
    // Verify the GDI negative-width reflection used by the native applications.
    const auto dc = CreateCompatibleDC(nullptr);
    const auto bitmap = CreateBitmap(4, 2, 1, 32, nullptr);
    if (!dc || !bitmap) {
        if (bitmap) {
            DeleteObject(bitmap);
        }
        if (dc) {
            DeleteDC(dc);
        }
        return 4;
    }
    const auto old = SelectObject(dc, bitmap);
    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = 2;
    info.bmiHeader.biHeight = -1;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    const unsigned char pixels[]{0, 0, 255, 0, 255, 0, 0, 0}; // red then blue, BGRX
    StretchDIBits(dc, 3, 0, -4, 2, 0, 0, 2, 1, pixels, &info, DIB_RGB_COLORS, SRCCOPY);
    const bool reflected =
        GetPixel(dc, 0, 0) == RGB(0, 0, 255) && GetPixel(dc, 3, 0) == RGB(255, 0, 0);
    if (!reflected) {
        std::cerr << "GDI pixels: " << GetPixel(dc, 0, 0) << ',' << GetPixel(dc, 1, 0) << ','
                  << GetPixel(dc, 2, 0) << ',' << GetPixel(dc, 3, 0) << '\n';
    }
    SelectObject(dc, old);
    DeleteObject(bitmap);
    DeleteDC(dc);
    if (!reflected) {
        return 5;
    }
#endif
    std::cout << "Mirrored preview and inverse pointer transform passed\n";
}
