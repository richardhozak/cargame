#include "register_types.h"

#include <gdextension_interface.h>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "car_physics_input.hpp"
#include "car_physics_simulation_info.hpp"
#include "car_physics_track_mesh.hpp"
#include "car_physics.hpp"

using namespace godot;

void initialize_physics_module(ModuleInitializationLevel p_level)
{
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
    {
        return;
    }

    UtilityFunctions::print("Physics Init");
    ClassDB::register_class<CarPhysicsInput>();
    ClassDB::register_class<CarPhysicsSimulationInfo>();
    ClassDB::register_class<CarPhysicsTrackMesh>();
    ClassDB::register_class<CarPhysics>();
}

void uninitialize_physics_module(ModuleInitializationLevel p_level)
{
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
    {
        return;
    }
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
