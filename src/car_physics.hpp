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
    std::unique_ptr<physics::Physics> physics;

protected:
    static void _bind_methods();

public:
    void _ready() override;

    void simulate(const Ref<CarPhysicsInput>& input);
    PackedByteArray save_state() const;
    void load_state(const PackedByteArray& state);
    size_t checkpoint_count() const;
    size_t collected_checkpoint_count() const;
};

}  // namespace godot
