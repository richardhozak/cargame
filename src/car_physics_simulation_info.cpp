#include "car_physics_simulation_info.hpp"

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>

#include <cassert>

using namespace godot;

void CarPhysicsSimulationInfo::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("get_step"), &CarPhysicsSimulationInfo::get_step);
    ClassDB::bind_method(D_METHOD("set_step", "p_step"), &CarPhysicsSimulationInfo::set_step);
    ClassDB::add_property("CarPhysicsSimulationInfo", PropertyInfo(Variant::INT, "step"), "set_step", "get_step");

    ClassDB::bind_method(D_METHOD("get_input"), &CarPhysicsSimulationInfo::get_input);
    ClassDB::bind_method(D_METHOD("set_input", "p_input"), &CarPhysicsSimulationInfo::set_input);
    ClassDB::add_property("CarPhysicsSimulationInfo", PropertyInfo(Variant::OBJECT, "input"), "set_input", "get_input");

    ClassDB::bind_method(D_METHOD("get_input_simulated"), &CarPhysicsSimulationInfo::get_input_simulated);
    ClassDB::bind_method(D_METHOD("set_input_simulated", "p_input_simulated"), &CarPhysicsSimulationInfo::set_input_simulated);
    ClassDB::add_property("CarPhysicsSimulationInfo", PropertyInfo(Variant::BOOL, "input_simulated"), "set_input_simulated", "get_input_simulated");

    ClassDB::bind_method(D_METHOD("get_speed"), &CarPhysicsSimulationInfo::get_speed);
    ClassDB::bind_method(D_METHOD("set_speed", "p_speed"), &CarPhysicsSimulationInfo::set_speed);
    ClassDB::add_property("CarPhysicsSimulationInfo", PropertyInfo(Variant::FLOAT, "speed"), "set_speed", "get_speed");

    ClassDB::bind_method(D_METHOD("get_rpm"), &CarPhysicsSimulationInfo::get_rpm);
    ClassDB::bind_method(D_METHOD("set_rpm", "p_rpm"), &CarPhysicsSimulationInfo::set_rpm);
    ClassDB::add_property("CarPhysicsSimulationInfo", PropertyInfo(Variant::FLOAT, "rpm"), "set_rpm", "get_rpm");

    ClassDB::bind_method(D_METHOD("get_gear"), &CarPhysicsSimulationInfo::get_gear);
    ClassDB::bind_method(D_METHOD("set_gear", "p_gear"), &CarPhysicsSimulationInfo::set_gear);
    ClassDB::add_property("CarPhysicsSimulationInfo", PropertyInfo(Variant::INT, "gear"), "set_gear", "get_gear");
}

CarPhysicsSimulationInfo::CarPhysicsSimulationInfo()
{
}

void CarPhysicsSimulationInfo::set_step(const int64_t& p_step)
{
    step = p_step;
}
int64_t CarPhysicsSimulationInfo::get_step() const
{
    return step;
}

void CarPhysicsSimulationInfo::set_input(const Ref<CarPhysicsInput>& p_input)
{
    input = p_input;
}
Ref<CarPhysicsInput> CarPhysicsSimulationInfo::get_input() const
{
    return input;
}

void CarPhysicsSimulationInfo::set_input_simulated(const bool& p_input_simulated)
{
    input_simulated = p_input_simulated;
}
bool CarPhysicsSimulationInfo::get_input_simulated() const
{
    return input_simulated;
}

void CarPhysicsSimulationInfo::set_speed(const float& p_speed)
{
    speed = p_speed;
}
float CarPhysicsSimulationInfo::get_speed() const
{
    return speed;
}

void CarPhysicsSimulationInfo::set_rpm(const float& p_rpm)
{
    rpm = p_rpm;
}
float CarPhysicsSimulationInfo::get_rpm() const
{
    return rpm;
}

void CarPhysicsSimulationInfo::set_gear(const int64_t& p_gear)
{
    gear = p_gear;
}
int64_t CarPhysicsSimulationInfo::get_gear() const
{
    return gear;
}