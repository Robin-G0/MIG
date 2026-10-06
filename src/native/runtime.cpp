#include "runtime.hpp"
#include <stdexcept>
#ifdef _WIN32
#include <mfapi.h>
#include <objbase.h>
#endif
namespace mig::native {
CaptureRuntime::CaptureRuntime() {
#ifdef _WIN32
    const auto com = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(com) && com != RPC_E_CHANGED_MODE) {
        throw std::runtime_error("Cannot initialize capture COM");
    }
    com_owned_ = SUCCEEDED(com);
    if (FAILED(MFStartup(MF_VERSION))) {
        if (com_owned_) {
            CoUninitialize();
        }
        throw std::runtime_error("Cannot initialize Media Foundation capture");
    }
#endif
}
CaptureRuntime::~CaptureRuntime() {
#ifdef _WIN32
    MFShutdown();
    if (com_owned_) {
        CoUninitialize();
    }
#endif
}
} // namespace mig::native
