#include "../../../src/apps/paint_buffer.hpp"

int main() {
    const auto compatible = GetDC(nullptr);
    if (!compatible) {
        return 1;
    }
    const auto before = GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS);
    bool valid = true;
    {
        mig::app::PaintBuffer buffer;
        valid &= buffer.resize(compatible, 640, 480);
        const auto bitmap = GetCurrentObject(buffer.dc(), OBJ_BITMAP);
        valid &= buffer.resize(compatible, 640, 480);
        valid &= bitmap == GetCurrentObject(buffer.dc(), OBJ_BITMAP);
        valid &= !buffer.resize(compatible, 0, 480);
        for (int index = 0; index < 30; ++index) {
            valid &= buffer.resize(compatible, 640 + index, 480 + index);
        }
    }
    valid &= GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS) == before;
    ReleaseDC(nullptr, compatible);
    return valid ? 0 : 1;
}
