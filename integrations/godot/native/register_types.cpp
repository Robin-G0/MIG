#include "mig_tracker.hpp"
#include <godot_cpp/godot.hpp>

namespace {
void initialize(godot::ModuleInitializationLevel level) {
    if (level == godot::MODULE_INITIALIZATION_LEVEL_SCENE) {
        godot::ClassDB::register_class<mig::godot_bridge::MigTrackerNative>();
    }
}
void terminate(godot::ModuleInitializationLevel) {}
} // namespace
extern "C" {
GDExtensionBool GDE_EXPORT mig_godot_init(GDExtensionInterfaceGetProcAddress get_proc_address,
                                          GDExtensionClassLibraryPtr library,
                                          GDExtensionInitialization* initialization) {
    godot::GDExtensionBinding::InitObject binding(get_proc_address, library, initialization);
    binding.register_initializer(initialize);
    binding.register_terminator(terminate);
    binding.set_minimum_library_initialization_level(godot::MODULE_INITIALIZATION_LEVEL_SCENE);
    return binding.init();
}
}
