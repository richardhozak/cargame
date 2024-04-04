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

void GDPhysics::_bind_methods()
{
    UtilityFunctions::print("bind methods");

    ClassDB::bind_method(D_METHOD("get_amplitude"), &GDPhysics::get_amplitude);
    ClassDB::bind_method(D_METHOD("set_amplitude", "p_amplitude"), &GDPhysics::set_amplitude);
    ClassDB::add_property("GDPhysics", PropertyInfo(Variant::FLOAT, "amplitude"), "set_amplitude", "get_amplitude");

    ClassDB::bind_method(D_METHOD("get_label"), &GDPhysics::get_label);
    ClassDB::bind_method(D_METHOD("set_label", "p_label"), &GDPhysics::set_label);
    ClassDB::add_property("GDPhysics", PropertyInfo(Variant::NODE_PATH, "label", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Label"), "set_label", "get_label");

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

    ClassDB::bind_method(D_METHOD("get_track"), &GDPhysics::get_track);
    ClassDB::bind_method(D_METHOD("set_track", "p_track"), &GDPhysics::set_track);
    ClassDB::add_property("GDPhysics", PropertyInfo(Variant::NODE_PATH, "track", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Node3D"), "set_track", "get_track");
}

GDPhysics::GDPhysics()
{
    UtilityFunctions::print("ctor");
    // Initialize any variables here.
    time_passed = 0.0;
    amplitude = 10.0;
}

GDPhysics::~GDPhysics()
{
    UtilityFunctions::print("dtor");
    // Add your cleanup here.
}

void GDPhysics::set_amplitude(const double p_amplitude)
{
    amplitude = p_amplitude;
}

double GDPhysics::get_amplitude() const
{
    return amplitude;
}

void GDPhysics::set_label(const NodePath& p_label)
{
    if (label_path == p_label)
    {
        return;
    }

    label_path = p_label;
}

NodePath GDPhysics::get_label() const
{
    return label_path;
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

// void GDPhysics::initialize()
// {
//     UtilityFunctions::print("initialize");

//     Node* node = get_node_or_null(track);

//     if (!node)
//     {
//         UtilityFunctions::printerr("could not get track node");
//         return;
//     }

//     Node3D* track_node = cast_to<Node3D>(node);

//     const TypedArray<Node> children = track_node->get_children(true);

//     // Vector3 scale = track_node->get_scale();
//     // (void)scale;  // TODO: use scale in track.scale

//     physics::Track track;
//     // track.scale = physics::Vector3{1.0, 1.0, 1.0};
//     track.scale = physics::Vector3{10.0, 10.0, 10.0};
//     // track.scale = physics::Vector3{scale.x, scale.y, scale.z};

//     for (int64_t child_idx = 0; child_idx < children.size(); ++child_idx)
//     {
//         Object* child_obj = static_cast<Object*>(children[child_idx]);
//         MeshInstance3D* child_mesh = cast_to<MeshInstance3D>(child_obj);
//         Node3D* child_node = cast_to<Node3D>(child_obj);
//         UtilityFunctions::print("child ", child_idx, " ", children[child_idx], " ", static_cast<bool>(child_mesh));
//         if (child_mesh)
//         {
//             Ref<Mesh> mesh = child_mesh->get_mesh();
//             for (int32_t surface_idx = 0; surface_idx < mesh->get_surface_count(); ++surface_idx)
//             {
//                 Ref<Material> surface_material = mesh->surface_get_material(surface_idx);
//                 BaseMaterial3D* material = cast_to<BaseMaterial3D>(surface_material.ptr());

//                 physics::MeshKind mesh_kind = mesh_kind_from_color(material->get_albedo());
//                 if (mesh_kind != physics::MeshKind::Road)
//                 {
//                     child_node->set_visible(false);
//                 }

//                 Array surface_arrays = mesh->surface_get_arrays(surface_idx);
//                 PackedVector3Array vertices = static_cast<PackedVector3Array>(surface_arrays[Mesh::ARRAY_VERTEX]);
//                 PackedInt32Array indices = static_cast<PackedInt32Array>(surface_arrays[Mesh::ARRAY_INDEX]);

//                 Vector3 scale = child_node->get_scale();
//                 UtilityFunctions::print("scale ", scale);

//                 Vector3 position = child_node->get_position();
//                 Quaternion rotation = child_node->get_quaternion();
//                 if (rotation.y == 1.0)
//                 {
//                     rotation.y = -1.0;
//                     rotation.w = -rotation.w;
//                 }

//                 physics::Mesh physics_mesh;

//                 physics_mesh.kind = mesh_kind;
//                 physics_mesh.scale = physics::Vector3{scale.x, scale.y, scale.z};
//                 physics_mesh.position = physics::Vector3{position.x, position.y, position.z};
//                 physics_mesh.rotation = physics::Vector4{rotation.x, rotation.y, rotation.z, rotation.w};

//                 for (Vector3 vertex : vertices)
//                 {
//                     physics_mesh.vertices.push_back({vertex.x, vertex.y, vertex.z});
//                 }

//                 for (int32_t index : indices)
//                 {
//                     physics_mesh.indices.push_back(index);
//                 }

//                 track.meshes.push_back(std::move(physics_mesh));
//             }
//         }
//         else
//         {
//             UtilityFunctions::printerr("does not have mesh");
//         }
//     }

//     // track_node->set_scale(Vector3{10.0, 10.0, 10.0});
//     UtilityFunctions::print("initialized", track.meshes.size());
//     physics::Configuration cfg;
//     physics = physics::new_physics(track, cfg);
// }

void GDPhysics::_notification(int p_what)
{
    switch (p_what)
    {
        case NOTIFICATION_EXTENSION_RELOADED:
            UtilityFunctions::print("reloaded");
            break;
    }
}

void GDPhysics::_ready()
{
    UtilityFunctions::print("ready", get_parent());
    GDPhysicsMesh* mesh = Object::cast_to<GDPhysicsMesh>(static_cast<Object*>(get_parent()->get_meta("mesh")));
    UtilityFunctions::print(mesh->meshes.size());

    physics::Configuration cfg;
    physics::Track track;
    track.scale = physics::Vector3{10.0, 10.0, 10.0};
    track.meshes = mesh->meshes;
    physics = physics::new_physics(track, cfg);
}

void GDPhysics::_process(double delta)
{
    time_passed += delta;

    Vector2 new_position = Vector2(
        amplitude + (amplitude * sin(time_passed * 2.0)),
        amplitude + (amplitude * cos(time_passed * 1.5)));

    if (Node* label_node = get_node_or_null(label_path))
    {
        Label* label = cast_to<Label>(label_node);
        if (label)
        {
            label->set_position(new_position);
            label->set_text(String::num_real(time_passed));
        }
    }
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

void GDPhysics::_physics_process(double delta)
{
    Input* input = Input::get_singleton();

    if (physics)
    {
        physics::Input physics_input{
            .up = input->is_key_pressed(KEY_UP),
            .down = input->is_key_pressed(KEY_DOWN),
            .left = input->is_key_pressed(KEY_LEFT),
            .right = input->is_key_pressed(KEY_RIGHT),
            .brake = input->is_key_pressed(KEY_Z),
            .restart = input->is_key_pressed(KEY_BACKSPACE),
            .respawn = input->is_key_pressed(KEY_ENTER),
        };

        physics::State state = physics->simulate(physics_input);
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

void GDPhysics::set_track(const NodePath& p_track)
{
    UtilityFunctions::print("set_track", p_track);
    track = p_track;
}
NodePath GDPhysics::get_track() const
{
    return track;
}
