#pragma once

#include <godot_cpp/classes/node3d.hpp>

#include <godot_cpp/variant/packed_byte_array.hpp>
#include <memory>

#include "car_physics_input.hpp"
#include "physics.h"

namespace godot {

class CarPhysics : public Node3D
{
    GDCLASS(CarPhysics, Node3D)
private:
    NodePath wheel1;
    NodePath wheel2;
    NodePath wheel3;
    NodePath wheel4;
    NodePath body;
    std::unique_ptr<physics::Physics> physics;
    physics::State last_state;

protected:
    static void _bind_methods();

public:
    enum CarPhysicsInputAction
    {
        NOOP,
        SAVE,
        RESET,
    };

    void _ready() override;

    CarPhysicsInputAction simulate(const Ref<CarPhysicsInput>& input);
    PackedByteArray save_state() const;
    void load_state(const PackedByteArray& state);

    void set_wheel1(const NodePath& p_wheel1);
    NodePath get_wheel1() const;

    void set_wheel2(const NodePath& p_wheel2);
    NodePath get_wheel2() const;

    void set_wheel3(const NodePath& p_wheel3);
    NodePath get_wheel3() const;

    void set_wheel4(const NodePath& p_wheel4);
    NodePath get_wheel4() const;

    void set_body(const NodePath& p_body);
    NodePath get_body() const;
};

}  // namespace godot

VARIANT_ENUM_CAST(CarPhysics::CarPhysicsInputAction);