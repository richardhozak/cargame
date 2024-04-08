#ifndef GDPHYSICS_H
#define GDPHYSICS_H

#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/quaternion.hpp>

#include "physics.h"

namespace godot {

class GDPhysicsMesh : public RefCounted
{
    GDCLASS(GDPhysicsMesh, RefCounted)
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

class PhysicsInput : public RefCounted
{
    GDCLASS(PhysicsInput, RefCounted)
public:
    static void _bind_methods();
    PhysicsInput();

    void set_up(const bool& p_up);
    bool get_up() const;

    void set_down(const bool& p_down);
    bool get_down() const;

    void set_left(const bool& p_left);
    bool get_left() const;

    void set_right(const bool& p_right);
    bool get_right() const;

    void set_brake(const bool& p_brake);
    bool get_brake() const;

    void set_restart(const bool& p_restart);
    bool get_restart() const;

    void set_respawn(const bool& p_respawn);
    bool get_respawn() const;

    const physics::Input& as_physics_input() const;

private:
    physics::Input input;
};

class GDPhysics : public Node3D
{
    GDCLASS(GDPhysics, Node3D)
private:
    NodePath wheel1;
    NodePath wheel2;
    NodePath wheel3;
    NodePath wheel4;
    NodePath body;
    std::unique_ptr<physics::Physics> physics;
    physics::State last_state;

protected:
public:
    static void _bind_methods();

    void _ready() override;

    void simulate(const Ref<PhysicsInput>& input);

    void set_wheel1(const NodePath& p_wheel1);
    NodePath get_wheel1() const;

    void set_wheel2(const NodePath& p_wheel2);
    NodePath get_wheel2() const;

    void set_wheel3(const NodePath& p_wheel3);
    NodePath get_wheel3() const;

    void set_wheel4(const NodePath& p_wheel4);
    NodePath get_wheel4() const;

    void set_body(const NodePath& p_body);
    NodePath get_body() const;
};

}  // namespace godot

#endif
