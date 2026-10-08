#include "runtime.hpp"
int main() {
    for (int cycle = 0; cycle < 3; ++cycle) {
        mig::native::CaptureRuntime runtime;
        // Nested ownership balances process-wide MF and per-thread COM references.
        mig::native::CaptureRuntime nested;
    }
}
