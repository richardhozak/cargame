#include "car_physics_step.hpp"

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>

#include <cassert>

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