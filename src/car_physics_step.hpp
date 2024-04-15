#pragma once

#include <godot_cpp/classes/ref_counted.hpp>

#include "car_physics_input.hpp"

#include <cstddef>

namespace godot {

class CarPhysicsStep : public RefCounted
{
    GDCLASS(CarPhysicsStep, RefCounted)
public:
    static void _bind_methods();
    CarPhysicsStep();

    void set_step(const int64_t& p_step);
    int64_t get_step() const;

    void set_input(const Ref<CarPhysicsInput>& p_input);
    Ref<CarPhysicsInput> get_input() const;

    void set_input_simulated(const bool& p_input_simulated);
    bool get_input_simulated() const;

    void set_speed(const float& p_speed);
    float get_speed() const;

    void set_rpm(const float& p_rpm);
    float get_rpm() const;

    void set_gear(const int64_t& p_gear);
    int64_t get_gear() const;

private:
    int64_t step;
    Ref<CarPhysicsInput> input;
    bool input_simulated;
    float speed;
    float rpm;
    int64_t gear;
};

}  // namespace godot