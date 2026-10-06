#include "scrollbars.hpp"
#include <commctrl.h>
#include <memory>

namespace mig::app {
namespace {
constexpr UINT_PTR subclass_id = 41;
struct ScrollbarState {
    ScrollbarColors colors;
};
void fill(HDC dc, RECT rectangle, COLORREF color) {
    const auto brush = CreateSolidBrush(color);
    FillRect(dc, &rectangle, brush);
    DeleteObject(brush);
}
void arrow(HDC dc, RECT rectangle, bool vertical, bool forward, COLORREF color) {
    const auto pen = CreatePen(PS_SOLID, 1, color);
    const auto previous = SelectObject(dc, pen);
    const int x = (rectangle.left + rectangle.right) / 2;
    const int y = (rectangle.top + rectangle.bottom) / 2;
    const int direction = forward ? 1 : -1;
    if (vertical) {
        MoveToEx(dc, x - 3, y - direction, nullptr);
        LineTo(dc, x, y + direction * 2);
        LineTo(dc, x + 3, y - direction);
    } else {
        MoveToEx(dc, x - direction, y - 3, nullptr);
        LineTo(dc, x + direction * 2, y);
        LineTo(dc, x - direction, y + 3);
    }
    SelectObject(dc, previous);
    DeleteObject(pen);
}
bool paint_bar(HWND window, HDC dc, LONG object, ScrollbarColors colors, RECT& bounds) {
    SCROLLBARINFO info{sizeof(info)};
    if (!GetScrollBarInfo(window, object, &info) || (info.rgstate[0] & STATE_SYSTEM_INVISIBLE)) {
        return false;
    }
    RECT window_bounds{};
    GetWindowRect(window, &window_bounds);
    bounds = info.rcScrollBar;
    OffsetRect(&bounds, -window_bounds.left, -window_bounds.top);
    fill(dc, bounds, colors.track);
    const bool vertical = object == OBJID_VSCROLL;
    RECT first = bounds, last = bounds, thumb = bounds;
    if (vertical) {
        first.bottom = first.top + info.dxyLineButton;
        last.top = last.bottom - info.dxyLineButton;
        thumb.top = bounds.top + info.xyThumbTop;
        thumb.bottom = bounds.top + info.xyThumbBottom;
        InflateRect(&thumb, -3, -1);
    } else {
        first.right = first.left + info.dxyLineButton;
        last.left = last.right - info.dxyLineButton;
        thumb.left = bounds.left + info.xyThumbTop;
        thumb.right = bounds.left + info.xyThumbBottom;
        InflateRect(&thumb, -1, -3);
    }
    arrow(dc, first, vertical, false, colors.thumb);
    arrow(dc, last, vertical, true, colors.thumb);
    if (thumb.right > thumb.left && thumb.bottom > thumb.top) {
        POINT cursor{};
        GetCursorPos(&cursor);
        const bool hovered = PtInRect(&info.rcScrollBar, cursor);
        const auto brush = CreateSolidBrush(hovered ? colors.hover : colors.thumb);
        const auto old_brush = SelectObject(dc, brush);
        const auto old_pen = SelectObject(dc, GetStockObject(NULL_PEN));
        RoundRect(dc, thumb.left, thumb.top, thumb.right, thumb.bottom, 6, 6);
        SelectObject(dc, old_pen);
        SelectObject(dc, old_brush);
        DeleteObject(brush);
    }
    return true;
}
void repaint(HWND window, const ScrollbarState& state) {
    const auto dc = GetWindowDC(window);
    if (dc) {
        paint_scrollbars(window, dc, state.colors);
        ReleaseDC(window, dc);
    }
}
LRESULT CALLBACK scrollbar_procedure(HWND window, UINT message, WPARAM wp, LPARAM lp, UINT_PTR,
                                     DWORD_PTR data) {
    const auto state = reinterpret_cast<ScrollbarState*>(data);
    if (message == WM_NCDESTROY) {
        RemoveWindowSubclass(window, scrollbar_procedure, subclass_id);
        delete state;
        return DefSubclassProc(window, message, wp, lp);
    }
    const auto result = DefSubclassProc(window, message, wp, lp);
    DWORD_PTR current{};
    if (!GetWindowSubclass(window, scrollbar_procedure, subclass_id, &current)) {
        return result;
    }
    const auto active = reinterpret_cast<ScrollbarState*>(current);
    switch (message) {
    case WM_NCMOUSEMOVE: {
        TRACKMOUSEEVENT tracking{sizeof(tracking), TME_LEAVE | TME_NONCLIENT, window, 0};
        TrackMouseEvent(&tracking);
        repaint(window, *active);
        break;
    }
    case WM_NCPAINT:
    case WM_NCACTIVATE:
    case WM_PAINT:
    case WM_VSCROLL:
    case WM_HSCROLL:
    case WM_MOUSEWHEEL:
    case WM_MOUSEHWHEEL:
    case WM_NCMOUSELEAVE:
    case WM_NCLBUTTONDOWN:
    case WM_KEYDOWN:
    case WM_SIZE:
    case WM_STYLECHANGED:
        repaint(window, *active);
        break;
    case WM_PRINT:
        if (wp && (lp & PRF_NONCLIENT)) {
            paint_scrollbars(window, reinterpret_cast<HDC>(wp), active->colors);
        }
        break;
    }
    return result;
}
} // namespace
void paint_scrollbars(HWND window, HDC dc, ScrollbarColors colors) {
    RECT vertical{}, horizontal{};
    const bool has_vertical = paint_bar(window, dc, OBJID_VSCROLL, colors, vertical);
    const bool has_horizontal = paint_bar(window, dc, OBJID_HSCROLL, colors, horizontal);
    if (has_vertical && has_horizontal) {
        fill(dc, {vertical.left, horizontal.top, vertical.right, horizontal.bottom}, colors.track);
    }
}
void theme_scrollbars(HWND window, ScrollbarColors colors) {
    wchar_t name[32]{};
    GetClassNameW(window, name, 32);
    const auto style = GetWindowLongW(window, GWL_STYLE);
    const bool list = _wcsicmp(name, L"ListBox") == 0 || _wcsicmp(name, L"ComboLBox") == 0;
    const bool multiline = _wcsicmp(name, L"Edit") == 0 && (style & ES_MULTILINE);
    if (!list && !multiline && !(style & (WS_VSCROLL | WS_HSCROLL))) {
        return;
    }
    DWORD_PTR data{};
    if (GetWindowSubclass(window, scrollbar_procedure, subclass_id, &data)) {
        reinterpret_cast<ScrollbarState*>(data)->colors = colors;
    } else {
        auto state = std::make_unique<ScrollbarState>(ScrollbarState{colors});
        if (!SetWindowSubclass(window, scrollbar_procedure, subclass_id,
                               reinterpret_cast<DWORD_PTR>(state.get()))) {
            return;
        }
        state.release();
    }
    RedrawWindow(window, nullptr, nullptr, RDW_FRAME | RDW_INVALIDATE);
}
} // namespace mig::app
