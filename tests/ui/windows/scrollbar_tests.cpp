#include "../../../src/apps/scrollbars.hpp"
#include <iostream>

namespace {
void check(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}
void check_colors(HWND window, mig::app::ScrollbarColors colors) {
    RECT bounds{};
    GetWindowRect(window, &bounds);
    const auto screen = GetDC(nullptr);
    const auto memory = CreateCompatibleDC(screen);
    const auto bitmap =
        CreateCompatibleBitmap(screen, bounds.right - bounds.left, bounds.bottom - bounds.top);
    const auto previous = SelectObject(memory, bitmap);
    SendMessageW(window, WM_PRINT, reinterpret_cast<WPARAM>(memory), PRF_NONCLIENT);
    for (const auto object : {OBJID_VSCROLL, OBJID_HSCROLL}) {
        SCROLLBARINFO bar{sizeof(bar)};
        check(GetScrollBarInfo(window, object, &bar), "Missing native scrollbar geometry.");
        const bool vertical = object == OBJID_VSCROLL;
        const auto x = bar.rcScrollBar.left - bounds.left;
        const auto y = bar.rcScrollBar.top - bounds.top;
        const auto middle = (bar.xyThumbTop + bar.xyThumbBottom) / 2;
        const auto width = bar.rcScrollBar.right - bar.rcScrollBar.left;
        const auto height = bar.rcScrollBar.bottom - bar.rcScrollBar.top;
        check(GetPixel(memory, x + (vertical ? 1 : width / 2), y + (vertical ? height / 2 : 1)) ==
                  colors.track,
              "Scrollbar track did not inherit the theme.");
        const auto pixel = GetPixel(memory, x + (vertical ? width / 2 : middle),
                                    y + (vertical ? middle : height / 2));
        check(pixel == colors.thumb || pixel == colors.hover,
              "Scrollbar thumb did not inherit the theme.");
    }
    SelectObject(memory, previous);
    DeleteObject(bitmap);
    DeleteDC(memory);
    ReleaseDC(nullptr, screen);
}
} // namespace
int main() {
    HWND owner{};
    try {
        owner = CreateWindowW(L"STATIC", L"Scrollbar test", WS_OVERLAPPEDWINDOW, -10000, -10000,
                              240, 240, nullptr, nullptr, nullptr, nullptr);
        const auto list = CreateWindowW(
            L"LISTBOX", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | LBS_HASSTRINGS, 0, 0,
            180, 160, owner, nullptr, nullptr, nullptr);
        check(owner && list, "Cannot create scrollbar test controls.");
        for (int index = 0; index < 100; ++index) {
            SendMessageW(list, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"A long item"));
        }
        SendMessageW(list, LB_SETHORIZONTALEXTENT, 600, 0);
        ShowWindow(owner, SW_SHOWNOACTIVATE);
        const mig::app::ScrollbarColors dark{RGB(37, 37, 38), RGB(100, 105, 112),
                                             RGB(157, 157, 157)};
        const mig::app::ScrollbarColors light{RGB(243, 243, 243), RGB(170, 175, 182),
                                              RGB(102, 102, 102)};
        for (const auto colors : {dark, light, dark}) {
            mig::app::theme_scrollbars(list, colors);
            check_colors(list, colors);
        }
        const int before = GetScrollPos(list, SB_VERT);
        SendMessageW(list, WM_VSCROLL, SB_LINEDOWN, 0);
        check(GetScrollPos(list, SB_VERT) > before, "Themed scrollbar lost native scrolling.");
        check_colors(list, dark);
        DestroyWindow(owner);
        std::cout << "Native scrollbar colors, theme changes and scrolling passed.\n";
        return 0;
    } catch (const std::exception& error) {
        if (owner) {
            DestroyWindow(owner);
        }
        std::cerr << error.what() << '\n';
        return 1;
    }
}
