#include "import_review.hpp"
#include "app.hpp"

namespace mig::app {
namespace {
struct ReviewDialog {
    bool accepted{}, done{};
};
LRESULT CALLBACK review_proc(HWND window, UINT message, WPARAM wp, LPARAM lp) {
    auto* state = reinterpret_cast<ReviewDialog*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        state = static_cast<ReviewDialog*>(reinterpret_cast<CREATESTRUCTW*>(lp)->lpCreateParams);
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
    }
    if (state && ((message == WM_COMMAND && (LOWORD(wp) == IDOK || LOWORD(wp) == IDCANCEL)) ||
                  message == WM_CLOSE)) {
        state->accepted = message == WM_COMMAND && LOWORD(wp) == IDOK;
        state->done = true;
        return 0;
    }
    return DefWindowProcW(window, message, wp, lp);
}
} // namespace
bool App::review_import(const Configuration& proposed) {
    // Disable consent before opening a modal loop: timers can still run inside it.
    release_keys();
    keyboard_enabled = false;
    SetDlgItemTextW(window, Keys, L"Keyboard: Off");
    const auto instance = GetModuleHandleW(nullptr);
    WNDCLASSW cls{};
    cls.lpfnWndProc = review_proc;
    cls.hInstance = instance;
    cls.lpszClassName = L"MIGImportReview";
    cls.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    cls.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    RegisterClassW(&cls);
    ReviewDialog state;
    const auto dialog =
        CreateWindowExW(WS_EX_DLGMODALFRAME, cls.lpszClassName,
                        L"Review imported movement mappings", WS_CAPTION | WS_SYSMENU | WS_POPUP,
                        CW_USEDEFAULT, CW_USEDEFAULT, 760, 600, window, nullptr, instance, &state);
    if (!dialog) {
        throw std::runtime_error("Cannot create import review window");
    }
    auto text = wide(ui::configuration_review(proposed));
    std::wstring content;
    for (const auto character : text) {
        if (character == L'\n') {
            content += L'\r';
        }
        content += character;
    }
    const auto summary = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                                         WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL |
                                             ES_MULTILINE | ES_READONLY,
                                         12, 12, 720, 490, dialog, nullptr, instance, nullptr);
    if (!summary) {
        DestroyWindow(dialog);
        throw std::runtime_error("Cannot display import review");
    }
    SendMessageW(summary, EM_SETLIMITTEXT, 0, 0);
    if (!SetWindowTextW(summary, content.c_str()) ||
        std::size_t(GetWindowTextLengthW(summary)) != content.size()) {
        DestroyWindow(dialog);
        throw std::runtime_error("Cannot display complete import review");
    }
    for (const auto [id, label, x] : {std::tuple{IDOK, L"Import reviewed profile", 380},
                                      std::tuple{IDCANCEL, L"Cancel", 600}}) {
        CreateWindowExW(0, L"BUTTON", label,
                        WS_CHILD | WS_VISIBLE | WS_TABSTOP |
                            (id == IDCANCEL ? BS_DEFPUSHBUTTON : BS_PUSHBUTTON),
                        x, 520, id == IDOK ? 210 : 130, 30, dialog,
                        reinterpret_cast<HMENU>(INT_PTR(id)), instance, nullptr);
    }
    EnableWindow(window, FALSE);
    ShowWindow(dialog, SW_SHOW);
    SetFocus(GetDlgItem(dialog, IDCANCEL));
    MSG message{};
    while (!state.done) {
        const auto message_status = GetMessageW(&message, nullptr, 0, 0);
        if (message_status <= 0) {
            if (message_status == 0) {
                PostQuitMessage(int(message.wParam));
            }
            break;
        }
        if (!IsDialogMessageW(dialog, &message)) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }
    EnableWindow(window, TRUE);
    DestroyWindow(dialog);
    SetActiveWindow(window);
    return state.accepted;
}
} // namespace mig::app
