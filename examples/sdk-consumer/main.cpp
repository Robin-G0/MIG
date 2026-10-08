#include "example_usage.hpp"
int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: mig-sdk-example config.json\n";
        return 1;
    }
    try {
        auto engine = initialize_mig(argv[1]);
        mig::Frame frame;
        calibrate(engine, frame);
        demonstrate_path(engine, frame);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
