#pragma once
#include "app.hpp"
namespace mig::app::preview_drawing {
inline COLORREF blended(COLORREF color, COLORREF background, float opacity) {
    const auto channel = [&](int a, int b) { return BYTE(a * opacity + b * (1 - opacity)); };
    return RGB(channel(GetRValue(color), GetRValue(background)),
               channel(GetGValue(color), GetGValue(background)),
               channel(GetBValue(color), GetBValue(background)));
}
inline COLORREF constraint_color(ConstraintType type) {
    return type == ConstraintType::Required      ? RGB(48, 195, 135)
           : type == ConstraintType::Forbidden   ? RGB(242, 87, 105)
           : type == ConstraintType::Interaction ? RGB(184, 112, 245)
                                                 : RGB(255, 210, 60);
}
#ifdef MIG_NATIVE_HANDS
template <class Project> void draw_hands(HDC dc, const Snapshot& result, Project project) {
    if (!result.hands_active) {
        return;
    }
    const auto pen = CreatePen(PS_SOLID, 2, RGB(255, 210, 60));
    const auto previous = SelectObject(dc, pen);
    const auto brush = SelectObject(dc, GetStockObject(NULL_BRUSH));
    for (std::size_t h = 0; h < std::min(result.hands.count, std::size_t(2)); ++h) {
        const auto& hand = result.hands.hands[h];
        const auto position = [&](int joint) {
            const auto& point = hand.points[joint];
            return project(Vec2{point.x * result.pose.aspect, point.y});
        };
        for (int base : {1, 5, 9, 13, 17}) {
            auto point = position(0);
            MoveToEx(dc, point.x, point.y, nullptr);
            Ellipse(dc, point.x - 2, point.y - 2, point.x + 3, point.y + 3);
            for (int joint = base; joint < base + 4; ++joint) {
                point = position(joint);
                LineTo(dc, point.x, point.y);
                Ellipse(dc, point.x - 2, point.y - 2, point.x + 3, point.y + 3);
            }
        }
    }
    SelectObject(dc, brush);
    SelectObject(dc, previous);
    DeleteObject(pen);
}
#endif
} // namespace mig::app::preview_drawing
