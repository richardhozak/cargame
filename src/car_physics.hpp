#pragma once

#include <godot_cpp/classes/node3d.hpp>

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
public:
    static void _bind_methods();

    void _ready() override;

    void simulate(const Ref<CarPhysicsInput>& input);

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