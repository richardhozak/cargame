#include "car_physics_input.hpp"

using namespace godot;

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
