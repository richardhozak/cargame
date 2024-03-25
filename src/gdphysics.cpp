#include "gdphysics.h"

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/node_path.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void GDPhysics::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("get_amplitude"), &GDPhysics::get_amplitude);
    ClassDB::bind_method(D_METHOD("set_amplitude", "p_amplitude"), &GDPhysics::set_amplitude);
    ClassDB::add_property("GDPhysics", PropertyInfo(Variant::FLOAT, "amplitude"), "set_amplitude", "get_amplitude");

    ClassDB::bind_method(D_METHOD("get_label"), &GDPhysics::get_label);
    ClassDB::bind_method(D_METHOD("set_label", "p_label"), &GDPhysics::set_label);
    ClassDB::add_property("GDPhysics", PropertyInfo(Variant::NODE_PATH, "label", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Label"), "set_label", "get_label");
}

GDPhysics::GDPhysics()
{
    // Initialize any variables here.
    time_passed = 0.0;
    amplitude = 10.0;
}

GDPhysics::~GDPhysics()
{
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

void GDPhysics::_ready()
{
    UtilityFunctions::print("test");
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
