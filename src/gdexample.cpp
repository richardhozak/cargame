#include "gdexample.h"

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/node_path.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void GDExample::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("get_amplitude"), &GDExample::get_amplitude);
    ClassDB::bind_method(D_METHOD("set_amplitude", "p_amplitude"), &GDExample::set_amplitude);
    ClassDB::add_property("GDExample", PropertyInfo(Variant::FLOAT, "amplitude"), "set_amplitude", "get_amplitude");

    ClassDB::bind_method(D_METHOD("get_label"), &GDExample::get_label);
    ClassDB::bind_method(D_METHOD("set_label", "p_label"), &GDExample::set_label);
    ClassDB::add_property("GDExample", PropertyInfo(Variant::NODE_PATH, "label", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Label"), "set_label", "get_label");
}

GDExample::GDExample()
{
    // Initialize any variables here.
    time_passed = 0.0;
    amplitude = 10.0;
}

GDExample::~GDExample()
{
    // Add your cleanup here.
}

void GDExample::set_amplitude(const double p_amplitude)
{
    amplitude = p_amplitude;
}

double GDExample::get_amplitude() const
{
    return amplitude;
}

void GDExample::set_label(const NodePath& p_label)
{
    if (label_path == p_label)
    {
        return;
    }

    label_path = p_label;
}

NodePath GDExample::get_label() const
{
    return label_path;
}

void GDExample::_ready()
{
    UtilityFunctions::print("test");
}

void GDExample::_process(double delta)
{
    time_passed += delta;

    Vector2 new_position = Vector2(
        amplitude + (amplitude * sin(time_passed * 2.0)),
        amplitude + (amplitude * cos(time_passed * 1.5)));

    set_position(new_position);

    Label* label = get_node<Label>(label_path);
    if (label)
    {
        label->set_text(String::num_real(time_passed));
    }
}
