#include "example_usage.hpp"
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
