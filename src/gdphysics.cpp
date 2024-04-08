#include "gdphysics.h"
#include "configuration.h"
#include "physics.h"

#include <cstdlib>
#include <godot_cpp/classes/base_material3d.hpp>
#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/wrapped.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/variant/node_path.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/projection.hpp>
#include <godot_cpp/variant/quaternion.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <cassert>
#include <godot_cpp/variant/vector3.hpp>
#include <vector>

using namespace godot;

physics::MeshKind mesh_kind_from_color(const Color& color);

void GDPhysicsMesh::_bind_methods()
{
    UtilityFunctions::print("bind methods physics mesh");

    ClassDB::bind_method(D_METHOD("add_mesh", "position", "rotation", "scale", "vertices", "indices", "color"), &GDPhysicsMesh::add_mesh);
}

bool GDPhysicsMesh::add_mesh(
    Vector3 position,
    Quaternion rotation,
    Vector3 scale,
    PackedVector3Array vertices,
    PackedInt32Array indices,
    Color color)
{
    UtilityFunctions::print("add mesh", position, rotation, scale, color);

    physics::MeshKind kind = mesh_kind_from_color(color);

    physics::Mesh physics_mesh;

    physics_mesh.kind = kind;
    physics_mesh.scale = physics::Vector3{scale.x, scale.y, scale.z};
    physics_mesh.position = physics::Vector3{position.x, position.y, position.z};
    physics_mesh.rotation = physics::Vector4{rotation.x, rotation.y, rotation.z, rotation.w};

    for (Vector3 vertex : vertices)
    {
        physics_mesh.vertices.push_back({vertex.x, vertex.y, vertex.z});
    }

    for (int32_t index : indices)
    {
        physics_mesh.indices.push_back(index);
    }

    meshes.push_back(std::move(physics_mesh));

    return kind == physics::MeshKind::Road;
}

void PhysicsInput::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("get_up"), &PhysicsInput::get_up);
    ClassDB::bind_method(D_METHOD("set_up", "p_up"), &PhysicsInput::set_up);
    ClassDB::add_property("PhysicsInput", PropertyInfo(Variant::BOOL, "up"), "set_up", "get_up");

    ClassDB::bind_method(D_METHOD("get_down"), &PhysicsInput::get_down);
    ClassDB::bind_method(D_METHOD("set_down", "p_down"), &PhysicsInput::set_down);
    ClassDB::add_property("PhysicsInput", PropertyInfo(Variant::BOOL, "down"), "set_down", "get_down");

    ClassDB::bind_method(D_METHOD("get_left"), &PhysicsInput::get_left);
    ClassDB::bind_method(D_METHOD("set_left", "p_left"), &PhysicsInput::set_left);
    ClassDB::add_property("PhysicsInput", PropertyInfo(Variant::BOOL, "left"), "set_left", "get_left");

    ClassDB::bind_method(D_METHOD("get_right"), &PhysicsInput::get_right);
    ClassDB::bind_method(D_METHOD("set_right", "p_right"), &PhysicsInput::set_right);
    ClassDB::add_property("PhysicsInput", PropertyInfo(Variant::BOOL, "right"), "set_right", "get_right");

    ClassDB::bind_method(D_METHOD("get_brake"), &PhysicsInput::get_brake);
    ClassDB::bind_method(D_METHOD("set_brake", "p_brake"), &PhysicsInput::set_brake);
    ClassDB::add_property("PhysicsInput", PropertyInfo(Variant::BOOL, "brake"), "set_brake", "get_brake");

    ClassDB::bind_method(D_METHOD("get_restart"), &PhysicsInput::get_restart);
    ClassDB::bind_method(D_METHOD("set_restart", "p_restart"), &PhysicsInput::set_restart);
    ClassDB::add_property("PhysicsInput", PropertyInfo(Variant::BOOL, "restart"), "set_restart", "get_restart");

    ClassDB::bind_method(D_METHOD("get_respawn"), &PhysicsInput::get_respawn);
    ClassDB::bind_method(D_METHOD("set_respawn", "p_respawn"), &PhysicsInput::set_respawn);
    ClassDB::add_property("PhysicsInput", PropertyInfo(Variant::BOOL, "respawn"), "set_respawn", "get_respawn");
}

PhysicsInput::PhysicsInput()
{
    input.up = false;
    input.down = false;
    input.left = false;
    input.right = false;
    input.brake = false;
    input.restart = false;
    input.respawn = false;
}

void PhysicsInput::set_up(const bool& p_up)
{
    input.up = p_up;
}
bool PhysicsInput::get_up() const
{
    return input.up;
}

void PhysicsInput::set_down(const bool& p_down)
{
    input.down = p_down;
}
bool PhysicsInput::get_down() const
{
    return input.down;
}

void PhysicsInput::set_left(const bool& p_left)
{
    input.left = p_left;
}
bool PhysicsInput::get_left() const
{
    return input.left;
}

void PhysicsInput::set_right(const bool& p_right)
{
    input.right = p_right;
}
bool PhysicsInput::get_right() const
{
    return input.right;
}

void PhysicsInput::set_brake(const bool& p_brake)
{
    input.brake = p_brake;
}
bool PhysicsInput::get_brake() const
{
    return input.brake;
}

void PhysicsInput::set_restart(const bool& p_restart)
{
    input.restart = p_restart;
}
bool PhysicsInput::get_restart() const
{
    return input.restart;
}

void PhysicsInput::set_respawn(const bool& p_respawn)
{
    input.respawn = p_respawn;
}
bool PhysicsInput::get_respawn() const
{
    return input.respawn;
}

const physics::Input& PhysicsInput::as_physics_input() const
{
    return input;
}

void GDPhysics::_bind_methods()
{
    UtilityFunctions::print("bind methods");

    ClassDB::bind_method(D_METHOD("simulate", "p_input"), &GDPhysics::simulate);

    ClassDB::bind_method(D_METHOD("get_wheel1"), &GDPhysics::get_wheel1);
    ClassDB::bind_method(D_METHOD("set_wheel1", "p_wheel1"), &GDPhysics::set_wheel1);
    ClassDB::add_property("GDPhysics", PropertyInfo(Variant::NODE_PATH, "wheel1", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Node3D"), "set_wheel1", "get_wheel1");

    ClassDB::bind_method(D_METHOD("get_wheel2"), &GDPhysics::get_wheel2);
    ClassDB::bind_method(D_METHOD("set_wheel2", "p_wheel2"), &GDPhysics::set_wheel2);
    ClassDB::add_property("GDPhysics", PropertyInfo(Variant::NODE_PATH, "wheel2", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Node3D"), "set_wheel2", "get_wheel2");

    ClassDB::bind_method(D_METHOD("get_wheel3"), &GDPhysics::get_wheel3);
    ClassDB::bind_method(D_METHOD("set_wheel3", "p_wheel3"), &GDPhysics::set_wheel3);
    ClassDB::add_property("GDPhysics", PropertyInfo(Variant::NODE_PATH, "wheel3", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Node3D"), "set_wheel3", "get_wheel3");

    ClassDB::bind_method(D_METHOD("get_wheel4"), &GDPhysics::get_wheel4);
    ClassDB::bind_method(D_METHOD("set_wheel4", "p_wheel4"), &GDPhysics::set_wheel4);
    ClassDB::add_property("GDPhysics", PropertyInfo(Variant::NODE_PATH, "wheel4", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Node3D"), "set_wheel4", "get_wheel4");

    ClassDB::bind_method(D_METHOD("get_body"), &GDPhysics::get_body);
    ClassDB::bind_method(D_METHOD("set_body", "p_body"), &GDPhysics::set_body);
    ClassDB::add_property("GDPhysics", PropertyInfo(Variant::NODE_PATH, "body", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Node3D"), "set_body", "get_body");
}

physics::MeshKind mesh_kind_from_color(const Color& color)
{
    if (color.is_equal_approx(Color{1.0, 0.0, 0.0, 1.0}))
    {
        UtilityFunctions::print("finish");
        return physics::MeshKind::Finish;
    }

    if (color.is_equal_approx(Color{0.0, 1.0, 0.0, 1.0}))
    {
        UtilityFunctions::print("start");
        return physics::MeshKind::Start;
    }

    if (color.is_equal_approx(Color{0.0, 0.0, 1.0, 1.0}))
    {
        UtilityFunctions::print("checkpoint");
        return physics::MeshKind::Checkpoint;
    }

    UtilityFunctions::print("road ", color.r, " ", color.g, " ", color.b, " ", color.a);
    return physics::MeshKind::Road;
}

void GDPhysics::_ready()
{
    if (!get_parent()->has_meta("mesh"))
    {
        UtilityFunctions::printerr("does not have meta");
        return;
    }

    UtilityFunctions::print("ready", get_parent());
    GDPhysicsMesh* mesh = Object::cast_to<GDPhysicsMesh>(static_cast<Object*>(get_parent()->get_meta("mesh")));
    UtilityFunctions::print(mesh->meshes.size());

    physics::Configuration cfg;
    physics::Track track;
    track.scale = physics::Vector3{10.0, 10.0, 10.0};
    track.meshes = mesh->meshes;
    physics = physics::new_physics(track, cfg);
}

Transform3D physics_matrix_to_transform(const physics::Matrix4& mat)
{
    Projection projection{
        {mat.x_axis[0], mat.x_axis[1], mat.x_axis[2], mat.x_axis[3]},
        {mat.y_axis[0], mat.y_axis[1], mat.y_axis[2], mat.y_axis[3]},
        {mat.z_axis[0], mat.z_axis[1], mat.z_axis[2], mat.z_axis[3]},
        {mat.w_axis[0], mat.w_axis[1], mat.w_axis[2], mat.w_axis[3]}};

    return static_cast<Transform3D>(projection);
}

void GDPhysics::simulate(const Ref<PhysicsInput>& input)
{
    if (input.is_null())
    {
        UtilityFunctions::printerr("Input is null");
        return;
    }

    physics::State state = physics->simulate(input->as_physics_input());
    Node3D* body_node = get_node<Node3D>(body);
    Node3D* wheel1_node = get_node<Node3D>(wheel1);
    Node3D* wheel2_node = get_node<Node3D>(wheel2);
    Node3D* wheel3_node = get_node<Node3D>(wheel3);
    Node3D* wheel4_node = get_node<Node3D>(wheel4);
    body_node->set_transform(physics_matrix_to_transform(state.body_transform));
    wheel1_node->set_transform(physics_matrix_to_transform(state.wheel_transforms[0]));
    wheel2_node->set_transform(physics_matrix_to_transform(state.wheel_transforms[1]));
    wheel3_node->set_transform(physics_matrix_to_transform(state.wheel_transforms[2]));
    wheel4_node->set_transform(physics_matrix_to_transform(state.wheel_transforms[3]));
    if (!last_state.finished && state.finished)
    {
        UtilityFunctions::print("finished");
    }
    last_state = state;
}

void GDPhysics::set_wheel1(const NodePath& p_wheel1)
{
    UtilityFunctions::print("set_wheel1");
    wheel1 = p_wheel1;
}
NodePath GDPhysics::get_wheel1() const
{
    return wheel1;
}

void GDPhysics::set_wheel2(const NodePath& p_wheel2)
{
    UtilityFunctions::print("set_wheel2");
    wheel2 = p_wheel2;
}
NodePath GDPhysics::get_wheel2() const
{
    return wheel2;
}

void GDPhysics::set_wheel3(const NodePath& p_wheel3)
{
    UtilityFunctions::print("set_wheel3");
    wheel3 = p_wheel3;
}
NodePath GDPhysics::get_wheel3() const
{
    return wheel3;
}

void GDPhysics::set_wheel4(const NodePath& p_wheel4)
{
    UtilityFunctions::print("set_wheel4");
    wheel4 = p_wheel4;
}
NodePath GDPhysics::get_wheel4() const
{
    return wheel4;
}

void GDPhysics::set_body(const NodePath& p_body)
{
    UtilityFunctions::print("set_body");
    body = p_body;
}
NodePath GDPhysics::get_body() const
{
    return body;
}
