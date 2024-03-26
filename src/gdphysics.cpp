#include "gdphysics.h"

#include <cstdlib>
#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/wrapped.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/node_path.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <cassert>

using namespace godot;

void GDPhysics::_bind_methods()
{
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

void GDPhysics::initialize()
{
    UtilityFunctions::print("initialize");
    Node3D* track_node = get_node<Node3D>(track);
    const TypedArray<Node> children = track_node->get_children(true);

    TypedArray<PackedVector3Array> vertex_arrays;
    TypedArray<PackedInt32Array> index_arrays;

    for (int64_t child_idx = 0; child_idx < children.size(); ++child_idx)
    {
        Object* child_obj = static_cast<Object*>(children[child_idx]);
        MeshInstance3D* child_mesh = cast_to<MeshInstance3D>(child_obj);
        UtilityFunctions::print("child ", child_idx, " ", children[child_idx], " ", static_cast<bool>(child_mesh));
        if (child_mesh)
        {
            Ref<Mesh> mesh = child_mesh->get_mesh();
            for (int32_t surface_idx = 0; surface_idx < mesh->get_surface_count(); ++surface_idx)
            {
                Array surface_arrays = mesh->surface_get_arrays(surface_idx);
                PackedVector3Array vertices = static_cast<PackedVector3Array>(surface_arrays[Mesh::ARRAY_VERTEX]);
                PackedInt32Array indices = static_cast<PackedInt32Array>(surface_arrays[Mesh::ARRAY_INDEX]);

                vertex_arrays.append(vertices);
                index_arrays.append(indices);
            }
        }
        else
        {
            UtilityFunctions::printerr("does not have mesh");
        }
    }

    UtilityFunctions::print("initialized", vertex_arrays.size(), index_arrays.size());
}

void GDPhysics::_notification(int p_what)
{
    switch (p_what)
    {
        case NOTIFICATION_READY:
        case NOTIFICATION_EXTENSION_RELOADED:
            initialize();
            break;
    }
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
    UtilityFunctions::print("set_track");
    track = p_track;
}
NodePath GDPhysics::get_track() const
{
    return track;
}
