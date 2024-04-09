#include "car_physics_input.hpp"

using namespace godot;

void CarPhysicsInput::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("get_up"), &CarPhysicsInput::get_up);
    ClassDB::bind_method(D_METHOD("set_up", "p_up"), &CarPhysicsInput::set_up);
    ClassDB::add_property("CarPhysicsInput", PropertyInfo(Variant::BOOL, "up"), "set_up", "get_up");

    ClassDB::bind_method(D_METHOD("get_down"), &CarPhysicsInput::get_down);
    ClassDB::bind_method(D_METHOD("set_down", "p_down"), &CarPhysicsInput::set_down);
    ClassDB::add_property("CarPhysicsInput", PropertyInfo(Variant::BOOL, "down"), "set_down", "get_down");

    ClassDB::bind_method(D_METHOD("get_left"), &CarPhysicsInput::get_left);
    ClassDB::bind_method(D_METHOD("set_left", "p_left"), &CarPhysicsInput::set_left);
    ClassDB::add_property("CarPhysicsInput", PropertyInfo(Variant::BOOL, "left"), "set_left", "get_left");

    ClassDB::bind_method(D_METHOD("get_right"), &CarPhysicsInput::get_right);
    ClassDB::bind_method(D_METHOD("set_right", "p_right"), &CarPhysicsInput::set_right);
    ClassDB::add_property("CarPhysicsInput", PropertyInfo(Variant::BOOL, "right"), "set_right", "get_right");

    ClassDB::bind_method(D_METHOD("get_brake"), &CarPhysicsInput::get_brake);
    ClassDB::bind_method(D_METHOD("set_brake", "p_brake"), &CarPhysicsInput::set_brake);
    ClassDB::add_property("CarPhysicsInput", PropertyInfo(Variant::BOOL, "brake"), "set_brake", "get_brake");

    ClassDB::bind_method(D_METHOD("get_restart"), &CarPhysicsInput::get_restart);
    ClassDB::bind_method(D_METHOD("set_restart", "p_restart"), &CarPhysicsInput::set_restart);
    ClassDB::add_property("CarPhysicsInput", PropertyInfo(Variant::BOOL, "restart"), "set_restart", "get_restart");

    ClassDB::bind_method(D_METHOD("get_respawn"), &CarPhysicsInput::get_respawn);
    ClassDB::bind_method(D_METHOD("set_respawn", "p_respawn"), &CarPhysicsInput::set_respawn);
    ClassDB::add_property("CarPhysicsInput", PropertyInfo(Variant::BOOL, "respawn"), "set_respawn", "get_respawn");
}

CarPhysicsInput::CarPhysicsInput()
{
    input.up = false;
    input.down = false;
    input.left = false;
    input.right = false;
    input.brake = false;
    input.restart = false;
    input.respawn = false;
}

void CarPhysicsInput::set_up(const bool& p_up)
{
    input.up = p_up;
}
bool CarPhysicsInput::get_up() const
{
    return input.up;
}

void CarPhysicsInput::set_down(const bool& p_down)
{
    input.down = p_down;
}
bool CarPhysicsInput::get_down() const
{
    return input.down;
}

void CarPhysicsInput::set_left(const bool& p_left)
{
    input.left = p_left;
}
bool CarPhysicsInput::get_left() const
{
    return input.left;
}

void CarPhysicsInput::set_right(const bool& p_right)
{
    input.right = p_right;
}
bool CarPhysicsInput::get_right() const
{
    return input.right;
}

void CarPhysicsInput::set_brake(const bool& p_brake)
{
    input.brake = p_brake;
}
bool CarPhysicsInput::get_brake() const
{
    return input.brake;
}

void CarPhysicsInput::set_restart(const bool& p_restart)
{
    input.restart = p_restart;
}
bool CarPhysicsInput::get_restart() const
{
    return input.restart;
}

void CarPhysicsInput::set_respawn(const bool& p_respawn)
{
    input.respawn = p_respawn;
}
bool CarPhysicsInput::get_respawn() const
{
    return input.respawn;
}

const physics::Input& CarPhysicsInput::as_physics_input() const
{
    return input;
}
