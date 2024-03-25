#pragma once

#include <stdint.h>

namespace physics {

struct Configuration
{
    uint8_t start_countdown_seconds = 3;
    float max_roll_angle = 60.0f;
    float max_steering_angle = 30.0f;
    int collision_mode = 2;
    bool four_wheel_drive = false;
    bool anti_rollbar = true;
    bool limited_slip_differentials = true;
    float max_engine_torque = 500.0f;
    float clutch_strength = 10.0f;
    float front_suspension_min_length = 0.3f;
    float front_suspension_max_length = 0.5f;
    float front_suspension_frequency = 1.5f;
    float front_suspension_damping = 0.5f;
    float rear_suspension_min_length = 0.3f;
    float rear_suspension_max_length = 0.5f;
    float rear_suspension_frequency = 1.5f;
    float rear_suspension_damping = 0.5f;
    float max_handbrake_torque = 4000.0f;
    float max_brake_torque = 1500.0f;
    float body_mass = 1500.0f;
    float wheel_width = 0.1;
    float wheel_radius = 0.3;
    float wheel_mass = 20.0;
    float vehicle_length = 4.0;
    float vehicle_width = 1.8;
    float vehicle_height = 0.4;
};

}  // namespace physics