#pragma once
#include <windows.h>

namespace mig::app {
struct ScrollbarColors {
    COLORREF track, thumb, hover;
};
void theme_scrollbars(HWND window, ScrollbarColors colors);
void paint_scrollbars(HWND window, HDC dc, ScrollbarColors colors);
} // namespace mig::app
