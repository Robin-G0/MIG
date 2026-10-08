#include "pose.hpp"
#include <iostream>
#include <vector>
int main(int argc, char** argv) {
    try {
        if (argc != 2) {
            return 1;
        }
        std::vector<std::uint8_t> rgb(640 * 480 * 3);
        // Recreate and destroy tasks/module on the owner thread. Repetition can
        // reveal teardown races that a single process-exit smoke test misses.
        for (int cycle = 0; cycle < 10; ++cycle) {
            mig::native::Pose pose(argv[1], false, mig::native::PoseModel::Lite);
            for (int i = 1; i <= 3; ++i) {
                const auto frame = pose.infer(rgb, 640, 480, i, i);
                if (frame.sequence != std::uint64_t(i) || frame.timestamp_ms != i ||
                    frame.points[0].confidence != 0) {
                    return 2;
                }
            }
#ifdef MIG_NATIVE_HANDS
            pose.set_hands_enabled(true);
            pose.infer(rgb, 640, 480, 4, 4);
            if (pose.hand_frame().count != 0) {
                return 3;
            }
            pose.set_hands_enabled(false);
#endif
        }
        std::cout << "Linux native pose/hands inference and 10 repeated lifecycles passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
