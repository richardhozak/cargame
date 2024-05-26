#include "car_physics_transforms.hpp"

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>

#include <cassert>

using namespace godot;

void CarPhysicsTransforms::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("get_body"), &CarPhysicsTransforms::get_body);
    ClassDB::bind_method(D_METHOD("set_body", "p_body"), &CarPhysicsTransforms::set_body);
    ClassDB::add_property("CarPhysicsTransforms", PropertyInfo(Variant::TRANSFORM3D, "body"), "set_body", "get_body");

    ClassDB::bind_method(D_METHOD("get_wheel1"), &CarPhysicsTransforms::get_wheel1);
    ClassDB::bind_method(D_METHOD("set_wheel1", "p_wheel1"), &CarPhysicsTransforms::set_wheel1);
    ClassDB::add_property("CarPhysicsTransforms", PropertyInfo(Variant::TRANSFORM3D, "wheel1"), "set_wheel1", "get_wheel1");

    ClassDB::bind_method(D_METHOD("get_wheel2"), &CarPhysicsTransforms::get_wheel2);
    ClassDB::bind_method(D_METHOD("set_wheel2", "p_wheel2"), &CarPhysicsTransforms::set_wheel2);
    ClassDB::add_property("CarPhysicsTransforms", PropertyInfo(Variant::TRANSFORM3D, "wheel2"), "set_wheel2", "get_wheel2");

    ClassDB::bind_method(D_METHOD("get_wheel3"), &CarPhysicsTransforms::get_wheel3);
    ClassDB::bind_method(D_METHOD("set_wheel3", "p_wheel3"), &CarPhysicsTransforms::set_wheel3);
    ClassDB::add_property("CarPhysicsTransforms", PropertyInfo(Variant::TRANSFORM3D, "wheel3"), "set_wheel3", "get_wheel3");

    ClassDB::bind_method(D_METHOD("get_wheel4"), &CarPhysicsTransforms::get_wheel4);
    ClassDB::bind_method(D_METHOD("set_wheel4", "p_wheel4"), &CarPhysicsTransforms::set_wheel4);
    ClassDB::add_property("CarPhysicsTransforms", PropertyInfo(Variant::TRANSFORM3D, "wheel4"), "set_wheel4", "get_wheel4");
}

CarPhysicsTransforms::CarPhysicsTransforms()
{
}

void CarPhysicsTransforms::set_body(const Transform3D& p_body)
{
    body = p_body;
}
Transform3D CarPhysicsTransforms::get_body() const
{
    return body;
}

void CarPhysicsTransforms::set_wheel1(const Transform3D& p_wheel1)
{
    wheel1 = p_wheel1;
}
Transform3D CarPhysicsTransforms::get_wheel1() const
{
    return wheel1;
}

void CarPhysicsTransforms::set_wheel2(const Transform3D& p_wheel2)
{
    wheel2 = p_wheel2;
}
Transform3D CarPhysicsTransforms::get_wheel2() const
{
    return wheel2;
}

void CarPhysicsTransforms::set_wheel3(const Transform3D& p_wheel3)
{
    wheel3 = p_wheel3;
}
Transform3D CarPhysicsTransforms::get_wheel3() const
{
    return wheel3;
}

void CarPhysicsTransforms::set_wheel4(const Transform3D& p_wheel4)
{
    wheel4 = p_wheel4;
}
Transform3D CarPhysicsTransforms::get_wheel4() const
{
    return wheel4;
}
