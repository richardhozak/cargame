#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>

#include <memory>

#include "car_physics_input.hpp"
#include "physics.h"

namespace godot {

class CarPhysics : public Node
{
    GDCLASS(CarPhysics, Node)
private:
    std::unique_ptr<physics::Physics> physics;

protected:
    static void _bind_methods();
    void _notification(int32_t p_notification);

public:
    void simulate(const Ref<CarPhysicsInput>& input);
    PackedByteArray save_state() const;
    void load_state(const PackedByteArray& state);
    size_t checkpoint_count() const;
    size_t collected_checkpoint_count() const;
};

}  // namespace godot
