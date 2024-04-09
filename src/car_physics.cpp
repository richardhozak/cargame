#include "car_physics.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

#include "car_physics_track_mesh.hpp"
#include "configuration.h"
#include "physics.h"

using namespace godot;

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
