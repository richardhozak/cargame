#include "car_physics_step.hpp"

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>

using namespace godot;

void CarPhysicsStep::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("get_step"), &CarPhysicsStep::get_step);
    ClassDB::bind_method(D_METHOD("set_step", "p_step"), &CarPhysicsStep::set_step);
    ClassDB::add_property("CarPhysicsStep", PropertyInfo(Variant::INT, "step"), "set_step", "get_step");

    ClassDB::bind_method(D_METHOD("get_input"), &CarPhysicsStep::get_input);
    ClassDB::bind_method(D_METHOD("set_input", "p_input"), &CarPhysicsStep::set_input);
    ClassDB::add_property("CarPhysicsStep", PropertyInfo(Variant::OBJECT, "input"), "set_input", "get_input");

    ClassDB::bind_method(D_METHOD("get_simulated"), &CarPhysicsStep::get_simulated);
    ClassDB::bind_method(D_METHOD("set_simulated", "p_input"), &CarPhysicsStep::set_simulated);
    ClassDB::add_property("CarPhysicsStep", PropertyInfo(Variant::BOOL, "simulated"), "set_simulated", "get_simulated");

    ClassDB::bind_method(D_METHOD("get_speed"), &CarPhysicsStep::get_speed);
    ClassDB::bind_method(D_METHOD("set_speed", "p_speed"), &CarPhysicsStep::set_speed);
    ClassDB::add_property("CarPhysicsStep", PropertyInfo(Variant::FLOAT, "speed"), "set_speed", "get_speed");

    ClassDB::bind_method(D_METHOD("get_rpm"), &CarPhysicsStep::get_rpm);
    ClassDB::bind_method(D_METHOD("set_rpm", "p_rpm"), &CarPhysicsStep::set_rpm);
    ClassDB::add_property("CarPhysicsStep", PropertyInfo(Variant::FLOAT, "rpm"), "set_rpm", "get_rpm");

    ClassDB::bind_method(D_METHOD("get_gear"), &CarPhysicsStep::get_gear);
    ClassDB::bind_method(D_METHOD("set_gear", "p_gear"), &CarPhysicsStep::set_gear);
    ClassDB::add_property("CarPhysicsStep", PropertyInfo(Variant::INT, "gear"), "set_gear", "get_gear");

    ClassDB::bind_method(D_METHOD("get_just_finished"), &CarPhysicsStep::get_just_finished);
    ClassDB::bind_method(D_METHOD("set_just_finished", "p_just_finished"), &CarPhysicsStep::set_just_finished);
    ClassDB::add_property("CarPhysicsStep", PropertyInfo(Variant::BOOL, "just_finished"), "set_just_finished", "get_just_finished");

    ClassDB::bind_method(D_METHOD("get_available_checkpoints"), &CarPhysicsStep::get_available_checkpoints);
    ClassDB::bind_method(D_METHOD("set_available_checkpoints", "p_available_checkpoints"), &CarPhysicsStep::set_available_checkpoints);
    ClassDB::add_property("CarPhysicsStep", PropertyInfo(Variant::INT, "available_checkpoints"), "set_available_checkpoints", "get_available_checkpoints");

    ClassDB::bind_method(D_METHOD("get_collected_checkpoints"), &CarPhysicsStep::get_collected_checkpoints);
    ClassDB::bind_method(D_METHOD("set_collected_checkpoints", "p_collected_checkpoints"), &CarPhysicsStep::set_collected_checkpoints);
    ClassDB::add_property("CarPhysicsStep", PropertyInfo(Variant::INT, "collected_checkpoints"), "set_collected_checkpoints", "get_collected_checkpoints");

    ClassDB::bind_method(D_METHOD("get_transforms"), &CarPhysicsStep::get_transforms);
    ClassDB::bind_method(D_METHOD("set_transforms", "p_transforms"), &CarPhysicsStep::set_transforms);
    ClassDB::add_property("CarPhysicsStep", PropertyInfo(Variant::OBJECT, "transforms"), "set_transforms", "get_transforms");
}

CarPhysicsStep::CarPhysicsStep()
{
}

void CarPhysicsStep::set_step(const int64_t& p_step)
{
    step = p_step;
}
int64_t CarPhysicsStep::get_step() const
{
    return step;
}

void CarPhysicsStep::set_input(const Ref<CarPhysicsInput>& p_input)
{
    input = p_input;
}
Ref<CarPhysicsInput> CarPhysicsStep::get_input() const
{
    return input;
}

void CarPhysicsStep::set_simulated(const bool& p_simulated)
{
    simulated = p_simulated;
}
bool CarPhysicsStep::get_simulated() const
{
    return simulated;
}

void CarPhysicsStep::set_speed(const float& p_speed)
{
    speed = p_speed;
}
float CarPhysicsStep::get_speed() const
{
    return speed;
}

void CarPhysicsStep::set_rpm(const float& p_rpm)
{
    rpm = p_rpm;
}
float CarPhysicsStep::get_rpm() const
{
    return rpm;
}

void CarPhysicsStep::set_gear(const int64_t& p_gear)
{
    gear = p_gear;
}
int64_t CarPhysicsStep::get_gear() const
{
    return gear;
}

void CarPhysicsStep::set_just_finished(const bool& p_just_finished)
{
    just_finished = p_just_finished;
}
bool CarPhysicsStep::get_just_finished() const
{
    return just_finished;
}

void CarPhysicsStep::set_available_checkpoints(const int64_t& p_available_checkpoints)
{
    available_checkpoints = p_available_checkpoints;
}
int64_t CarPhysicsStep::get_available_checkpoints() const
{
    return available_checkpoints;
}

void CarPhysicsStep::set_collected_checkpoints(const int64_t& p_collected_checkpoints)
{
    collected_checkpoints = p_collected_checkpoints;
}
int64_t CarPhysicsStep::get_collected_checkpoints() const
{
    return collected_checkpoints;
}

void CarPhysicsStep::set_transforms(const Ref<CarPhysicsTransforms>& p_transforms)
{
    transforms = p_transforms;
}
Ref<CarPhysicsTransforms> CarPhysicsStep::get_transforms() const
{
    return transforms;
}
