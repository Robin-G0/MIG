#include "keyboard.hpp"
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>
#include <array>
#include <cstdlib>
namespace mig::linux_ui {
struct Keyboard::Impl {
    Display* display{};
    std::array<KeyCode, 256> held{};
    Impl() {
        // XTest cannot control native Wayland applications; never claim otherwise.
        if (!std::getenv("WAYLAND_DISPLAY")) {
            display = XOpenDisplay(nullptr);
        }
        if (display) {
            int event, error, major, minor;
            if (!XTestQueryExtension(display, &event, &error, &major, &minor)) {
                XCloseDisplay(display);
                display = nullptr;
            }
        }
    }
    ~Impl() {
        if (display) {
            for (const auto code : held) {
                if (code) {
                    XTestFakeKeyEvent(display, code, False, CurrentTime);
                }
            }
            XSync(display, False);
            XCloseDisplay(display);
        }
    }
};
namespace {
KeySym symbol(int key) {
    if (key >= 'A' && key <= 'Z') {
        return XK_a + key - 'A';
    }
    if (key >= '0' && key <= '9') {
        return XK_0 + key - '0';
    }
    if (key >= 112 && key <= 135) {
        return XK_F1 + key - 112;
    }
    switch (key) {
    case 8:
        return XK_BackSpace;
    case 9:
        return XK_Tab;
    case 13:
        return XK_Return;
    case 16:
        return XK_Shift_L;
    case 17:
        return XK_Control_L;
    case 18:
        return XK_Alt_L;
    case 27:
        return XK_Escape;
    case 32:
        return XK_space;
    case 33:
        return XK_Page_Up;
    case 34:
        return XK_Page_Down;
    case 35:
        return XK_End;
    case 36:
        return XK_Home;
    case 37:
        return XK_Left;
    case 38:
        return XK_Up;
    case 39:
        return XK_Right;
    case 40:
        return XK_Down;
    case 45:
        return XK_Insert;
    case 46:
        return XK_Delete;
    case 91:
        return XK_Super_L;
    case 92:
        return XK_Super_R;
    case 160:
        return XK_Shift_L;
    case 161:
        return XK_Shift_R;
    case 162:
        return XK_Control_L;
    case 163:
        return XK_Control_R;
    case 164:
        return XK_Alt_L;
    case 165:
        return XK_Alt_R;
    case 187:
        return XK_equal;
    case 189:
        return XK_minus;
    default:
        return NoSymbol;
    }
}
} // namespace
Keyboard::Keyboard() : impl_(std::make_unique<Impl>()) {}
Keyboard::~Keyboard() = default;
bool Keyboard::available() const {
    return impl_->display != nullptr;
}
bool Keyboard::key(int key, bool release) {
    if (!available() || key < 1 || key > 255) {
        return false;
    }
    const auto code = release ? impl_->held[key] : XKeysymToKeycode(impl_->display, symbol(key));
    if (!code) {
        return release;
    }
    if (!XTestFakeKeyEvent(impl_->display, code, release ? False : True, CurrentTime)) {
        return false;
    }
    impl_->held[key] = release ? 0 : code;
    XFlush(impl_->display);
    return true;
}
bool Keyboard::text(char16_t character) {
    if (!available() || (character >= 0xd800 && character <= 0xdfff)) {
        return false;
    }
    const KeySym desired = character < 256 ? character : 0x01000000u | character;
    const auto code = XKeysymToKeycode(impl_->display, desired);
    if (!code) {
        return false;
    }
    const bool shifted = XkbKeycodeToKeysym(impl_->display, code, 0, 0) != desired;
    if (shifted && XkbKeycodeToKeysym(impl_->display, code, 0, 1) != desired) {
        return false;
    }
    const auto shift = XKeysymToKeycode(impl_->display, XK_Shift_L);
    if (shifted) {
        XTestFakeKeyEvent(impl_->display, shift, True, CurrentTime);
    }
    XTestFakeKeyEvent(impl_->display, code, True, CurrentTime);
    XTestFakeKeyEvent(impl_->display, code, False, CurrentTime);
    if (shifted) {
        XTestFakeKeyEvent(impl_->display, shift, False, CurrentTime);
    }
    XFlush(impl_->display);
    return true;
}
} // namespace mig::linux_ui
