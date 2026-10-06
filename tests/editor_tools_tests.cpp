#include "../src/apps/editor_tools.hpp"
#include <iostream>

using namespace mig;
void check(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}
int main() {
    try {
        std::vector<SpatialConstraint> regions{
            {"first", 15, {8, 4}}, {"next", 15, {8, 3}}, {"right", 16, {8, 4}}};
        ui::RegionSelection selected;
        selected.begin({9.2f, 5.2f}, false);
        selected.update(regions, {7.8f, 2.8f}, 15);
        check(selected.ids.size() == 2 && selected.includes("first") && selected.includes("next"),
              "Reverse rectangular selection must include both wrist regions, not other layers");
        selected.dragging = false;
        selected.begin({8.5f, 4.5f}, false);
        selected.update(regions, {8.5f, 4.5f}, 15);
        check(selected.ids.size() == 1 && selected.includes("first"),
              "Click must replace selection");
        selected.begin({8.5f, 3.5f}, true);
        selected.update(regions, {8.5f, 3.5f}, 15);
        check(selected.ids.size() == 2, "Ctrl-click must preserve the previous selection");
        selected.begin({0, 0}, true);
        selected.update(regions, {1, 1}, 15);
        check(selected.ids.size() == 2, "Ctrl drag over empty space must preserve selection");
        selected.begin({0, 0}, false);
        selected.update(regions, {1, 1}, 15);
        check(selected.ids.empty(), "Empty rectangle must clear a replaced selection");
        ui::contour(regions, "first", "low_");
        check(ui::contour_target(regions, {7.5f, 4.5f}, 15) == "first",
              "Clicking tolerance must resolve its main region");
        check(ui::contour_target(regions, {8.5f, 3.5f}, 15) == "next",
              "Main regions take precedence over overlapping tolerances");
        check(ui::contour_target(regions, {0, 0}, 15).empty(), "Empty contour click has no target");
        auto brush = regions[0];
        brush.priority = Priority::Low;
        check(ui::tolerance_target(regions, brush, {8.5f, 5.5f}) == "first",
              "Low brush must find a matching main region without preselection");
        const auto before = regions;
        ui::erase_regions(regions, {"first"});
        check(regions.size() == 2 && regions[0].id == "next" && regions[1].id == "right",
              "Deleting a region must remove its linked contour and preserve other regions");
        ui::History<std::vector<SpatialConstraint>> history;
        history.commit(before);
        check(history.undo(regions) && regions == before,
              "Undo restores the region and its contour");
        check(history.redo(regions) && regions.size() == 2, "Redo reapplies the whole deletion");
        Motion input;
        input.fingers.push_back({});
        input.steps.push_back({"step"});
        input.steps[0].fingers.push_back({});
        SpatialConstraint purple{"purple", 15, {8, 0}, ConstraintType::Interaction};
        purple.interaction = Interaction{};
        purple.fingers.push_back({});
        input.steps[0].constraints.push_back(purple);
        ui::erase_regions(input.steps[0].constraints, {"purple"});
        check(input.fingers.size() == 1 && input.steps[0].fingers.size() == 1,
              "Deleting a purple region must preserve intentionally independent finger scopes");
        input.steps[0].constraints.push_back(purple);
        ui::clear_fingers(input);
        const auto counts = ui::binding_counts(input);
        check(counts.input + counts.steps + counts.cells == 0 && counts.interactions == 1,
              "Explicit finger reset clears all scopes without deleting signs or spatial regions");
        std::cout << "Editor selection, tolerance, deletion and scoped finger cleanup passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
