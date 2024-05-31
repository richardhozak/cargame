#pragma once

#include <godot_cpp/classes/ref_counted.hpp>

#include "car_physics_input.hpp"
#include "car_physics_transforms.hpp"

#include <cstdint>

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

    void set_simulated(const bool& p_simulated);
    bool get_simulated() const;

    void set_speed(const float& p_speed);
    float get_speed() const;

    void set_rpm(const float& p_rpm);
    float get_rpm() const;

    void set_gear(const int64_t& p_gear);
    int64_t get_gear() const;

    void set_just_finished(const bool& p_just_finished);
    bool get_just_finished() const;

    void set_available_checkpoints(const int64_t& p_available_checkpoints);
    int64_t get_available_checkpoints() const;

    void set_collected_checkpoints(const int64_t& p_collected_checkpoints);
    int64_t get_collected_checkpoints() const;

    void set_transforms(const Ref<CarPhysicsTransforms>& p_transforms);
    Ref<CarPhysicsTransforms> get_transforms() const;

private:
    int64_t step;
    Ref<CarPhysicsInput> input;
    bool simulated;
    float speed;
    float rpm;
    int64_t gear;
    bool just_finished;
    int64_t available_checkpoints;
    int64_t collected_checkpoints;
    Ref<CarPhysicsTransforms> transforms;
};

}  // namespace godot
