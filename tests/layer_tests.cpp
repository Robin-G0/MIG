#include "../src/apps/authoring.hpp"
#include <iostream>
using namespace mig;
int main() {
    try {
        Motion input;
        SpatialConstraint head{"head_start", 33, {2, 3}};
        head.order = 1;
        head.fingers = {{HandSide::Right, Finger::Index, FingerPose::Extended, 300, 100}};
        auto margin = head;
        margin.id = "margin";
        margin.priority = Priority::Low;
        margin.order = 0;
        margin.tolerance_for = head.id;
        auto sign = head;
        sign.id = "sign";
        sign.type = ConstraintType::Interaction;
        sign.order = 2;
        sign.interaction = Interaction{HandSide::Right, Gesture::OK, 5000};
        input.constraints = {{"guard", 33, {4, 5}, ConstraintType::Forbidden},
                             {"other", 15, {1, 1}}};
        input.steps = {{"first", StepMode::Ordered, {head, margin}},
                       {"second", StepMode::Ordered, {sign}}};
        input.recordings = {{33, {{2.f, 3.f}}}};
        const auto original = input;
        auto expected = original;
        expected.constraints[0].landmark = 16;
        for (auto& step : expected.steps) {
            for (auto& item : step.constraints) {
                item.landmark = 16;
            }
        }
        ui::reassign_layer(input, 33, 16);
        if (input != expected || ui::layers(input).size() != 2) {
            throw std::runtime_error("Reassign must preserve every setting and all other layers");
        }
        for (const auto target : {15, -1, 34}) {
            bool rejected = false;
            try {
                ui::reassign_layer(input, 16, target);
            } catch (const std::exception&) {
                rejected = true;
            }
            if (!rejected || input != expected) {
                throw std::runtime_error("Conflicts and invalid targets must fail atomically");
            }
        }
        ui::reassign_layer(input, 16, 16);
        if (input != expected) {
            throw std::runtime_error("Saving an unchanged layer must not alter it");
        }
        ui::reassign_layer(input, 16, 33);
        if (input != original) {
            throw std::runtime_error("Layer reassignment must be reversible without data loss");
        }
        std::cout << "Layer reassignment, settings preservation and conflicts passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
