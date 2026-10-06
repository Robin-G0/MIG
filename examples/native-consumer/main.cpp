#include <iostream>
#include <mig/native/pose.hpp>
#include <vector>
void demonstrate_inference(const std::string& runtime, bool hands) {
    mig::native::Pose estimator(runtime);
    estimator.set_hands_enabled(hands);
    // Blank RGB tests ownership/inference only. It cannot establish human accuracy.
    std::vector<std::uint8_t> rgb(640 * 480 * 3);
    const auto frame = estimator.infer(rgb, 640, 480, 1, 1);
    std::cout << "Game inference: sequence=" << frame.sequence
              << " hands-enabled=" << estimator.hands_enabled() << '\n';
    estimator.set_hands_enabled(false);
}

int main(int argc, char** argv) {
    if (argc < 2 || argc > 3 || (argc == 3 && std::string(argv[2]) != "--hands")) {
        std::cerr << "Usage: mig-native-example runtime_directory [--hands]\n";
        return 1;
    }
    try {
        demonstrate_inference(argv[1], argc == 3);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
