#pragma once

#include <godot_cpp/classes/ref_counted.hpp>

#include "physics.h"

namespace godot {

class PhysicsInput : public RefCounted
{
    GDCLASS(PhysicsInput, RefCounted)
public:
    static void _bind_methods();
    PhysicsInput();

    void set_up(const bool& p_up);
    bool get_up() const;

    void set_down(const bool& p_down);
    bool get_down() const;

    void set_left(const bool& p_left);
    bool get_left() const;

    void set_right(const bool& p_right);
    bool get_right() const;

    void set_brake(const bool& p_brake);
    bool get_brake() const;

    void set_restart(const bool& p_restart);
    bool get_restart() const;

    void set_respawn(const bool& p_respawn);
    bool get_respawn() const;

    const physics::Input& as_physics_input() const;

private:
    physics::Input input;
};

}  // namespace godot