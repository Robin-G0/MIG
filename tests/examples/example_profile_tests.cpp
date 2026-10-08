#include "../../examples/common/profile.hpp"
#include "../../examples/common/recognition.hpp"
#include "../../examples/common/synthetic.hpp"
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 2) {
        return 1;
    }
    try {
        auto configuration = mig::load_configuration(argv[1]);
        configuration.motions.at(0).action = "custom action";
        mig::Engine engine(configuration);
        const auto before = engine.configuration();
        std::string status;
        demo::import_profile(engine, "profile-that-does-not-exist.json", status);
        if (engine.configuration() != before || status.empty()) {
            throw std::runtime_error("Failed import changed the configuration");
        }
        bool hands_enabled{};
        demo::import_profile(engine, argv[1], status,
                             [&](bool enabled) { hands_enabled = enabled; });
        if (engine.configuration() != mig::load_configuration(argv[1])) {
            throw std::runtime_error("Valid import did not replace configuration");
        }
        if (!hands_enabled) {
            throw std::runtime_error("Imported profile did not enable required hands");
        }
        const auto imported = engine.configuration();
        demo::import_profile(engine, argv[1], status,
                             [](bool) { throw std::runtime_error("Hands runtime missing"); });
        if (engine.configuration() != imported || status != "Hands runtime missing") {
            throw std::runtime_error("Runtime failure did not preserve the old profile");
        }
        engine = mig::Engine(configuration);
        unsigned actions{};
        for (std::uint64_t sequence = 1; sequence <= 90; ++sequence) {
            mig::Frame body;
            mig::hands::Frame hands;
            demo::synthetic_frame(sequence, body, hands);
            actions += demo::consume(engine, body, &status);
        }
        if (actions != 2 || status.find("custom action") == std::string::npos ||
            status.find("Right hand raised!") == std::string::npos) {
            throw std::runtime_error("Generic or simultaneous actions missing from HUD");
        }
        std::cout << "Example profile imports and HUD events passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
