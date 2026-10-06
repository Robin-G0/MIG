#pragma once
#include <windows.h>

namespace mig::app {
class PaintBuffer {
public:
    PaintBuffer() = default;
    PaintBuffer(const PaintBuffer&) = delete;
    PaintBuffer& operator=(const PaintBuffer&) = delete;
    ~PaintBuffer() {
        if (dc_) {
            SelectObject(dc_, original_);
            DeleteObject(bitmap_);
            DeleteDC(dc_);
        }
    }
    bool resize(HDC compatible, int width, int height) {
        if (width == width_ && height == height_ && dc_) {
            return true;
        }
        if (width <= 0 || height <= 0) {
            return false;
        }
        if (!dc_) {
            dc_ = CreateCompatibleDC(compatible);
            if (!dc_) {
                return false;
            }
        }
        const auto replacement = CreateCompatibleBitmap(compatible, width, height);
        if (!replacement) {
            return false;
        }
        const auto previous = SelectObject(dc_, replacement);
        if (!previous || previous == HGDI_ERROR) {
            DeleteObject(replacement);
            return false;
        }
        if (!bitmap_) {
            original_ = previous;
        } else {
            DeleteObject(bitmap_);
        }
        bitmap_ = replacement;
        width_ = width;
        height_ = height;
        return true;
    }
    HDC dc() const {
        return dc_;
    }

private:
    HDC dc_{};
    HBITMAP bitmap_{};
    HGDIOBJ original_{};
    int width_{}, height_{};
};
} // namespace mig::app
