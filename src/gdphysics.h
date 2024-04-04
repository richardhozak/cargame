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

class GDPhysics : public Node3D
{
    GDCLASS(GDPhysics, Node3D)

private:
    double time_passed;
    double amplitude;
    NodePath label_path;
    NodePath wheel1;
    NodePath wheel2;
    NodePath wheel3;
    NodePath wheel4;
    NodePath body;
    NodePath track;
    std::unique_ptr<physics::Physics> physics;
    physics::State last_state;

    static const int NOTIFICATION_EXTENSION_RELOADED = 2;

    // void initialize();

protected:
public:
    static void _bind_methods();
    GDPhysics();
    ~GDPhysics();

    void _notification(int p_what);
    void _ready() override;
    void _process(double delta) override;
    void _physics_process(double delta) override;

    void set_amplitude(const double p_amplitude);
    double get_amplitude() const;

    void set_label(const NodePath& p_label);
    NodePath get_label() const;

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

    void set_track(const NodePath& p_track);
    NodePath get_track() const;
};

}  // namespace godot

#endif
