#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/quaternion.hpp>
#include <godot_cpp/variant/vector3.hpp>

#include "physics.h"

namespace godot {

class CarPhysicsTrackMesh : public RefCounted
{
    GDCLASS(CarPhysicsTrackMesh, RefCounted)
public:
    static void _bind_methods();
    bool add_mesh(
        Vector3 position,
        Quaternion rotation,
        Vector3 scale,
        PackedVector3Array vertices,
        PackedInt32Array indices,
        Color color);
    std::vector<physics::Mesh> meshes;
};

}  // namespace godot