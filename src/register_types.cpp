#include "register_types.h"

#include "gdphysics.h"

#include <gdextension_interface.h>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void initialize_physics_module(ModuleInitializationLevel p_level)
{
    UtilityFunctions::print("init ", p_level, " ", MODULE_INITIALIZATION_LEVEL_SCENE);

    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
    {
        return;
    }

    UtilityFunctions::print("register class");
    ClassDB::register_class<GDPhysics>();
    ClassDB::register_class<GDPhysicsMesh>();
    // GDPhysics::_bind_methods();
}

void uninitialize_physics_module(ModuleInitializationLevel p_level)
{
    UtilityFunctions::print("deinit", p_level);
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
    {
        return;
    }

    // ClassDB::deinitialize(static_cast<GDExtensionInitializationLevel>(p_level));
}

extern "C" {
// Initialization.
GDExtensionBool GDE_EXPORT physics_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization* r_initialization)
{
    godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

    init_obj.register_initializer(initialize_physics_module);
    init_obj.register_terminator(uninitialize_physics_module);
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

    return init_obj.init();
}
}
