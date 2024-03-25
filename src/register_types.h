#ifndef GDPHYSICS_REGISTER_TYPES_H
#define GDPHYSICS_REGISTER_TYPES_H

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void initialize_physics_module(ModuleInitializationLevel p_level);
void uninitialize_physics_module(ModuleInitializationLevel p_level);

#endif  // GDPHYSICS_REGISTER_TYPES_H
