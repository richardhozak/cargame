#include "car_physics.hpp"

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/core/property_info.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/time.hpp>

#include "car_physics_input.hpp"
#include "car_physics_step.hpp"
#include "car_physics_step.hpp"
#include "car_physics_track_mesh.hpp"
#include "car_physics_transforms.hpp"
#include "physics.h"

using namespace godot;

void CarPhysics::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("simulate", "p_input"), &CarPhysics::simulate);
    ClassDB::bind_method(D_METHOD("save_state"), &CarPhysics::save_state);
    ClassDB::bind_method(D_METHOD("load_state", "p_state"), &CarPhysics::load_state);
    ClassDB::bind_method(D_METHOD("checkpoint_count"), &CarPhysics::checkpoint_count);
    ClassDB::bind_method(D_METHOD("collected_checkpoint_count"), &CarPhysics::collected_checkpoint_count);

    ADD_SIGNAL(MethodInfo("simulated", PropertyInfo(Variant::OBJECT, "step", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "CarPhysicsStep")));
}

void CarPhysics::_notification(int32_t p_what)
{
    if (p_what != NOTIFICATION_READY)
    {
        return;
    }

    UtilityFunctions::print("CarPhysics ready");
    if (!get_parent()->has_meta("mesh"))
    {
        UtilityFunctions::printerr("does not have meta");
        return;
    }

    UtilityFunctions::print("ready", get_parent());
    CarPhysicsTrackMesh* mesh = Object::cast_to<CarPhysicsTrackMesh>(static_cast<Object*>(get_parent()->get_meta("mesh")));
    UtilityFunctions::print("mesh size", mesh->meshes.size());

    physics::Track track;
    track.scale = physics::Vector3{10.0, 10.0, 10.0};
    track.meshes = mesh->meshes;
    physics = physics::new_physics(track);
}

Transform3D physics_matrix_to_transform(const physics::Matrix4& mat)
{
    Projection projection{
        {mat.x_axis[0], mat.x_axis[1], mat.x_axis[2], mat.x_axis[3]},
        {mat.y_axis[0], mat.y_axis[1], mat.y_axis[2], mat.y_axis[3]},
        {mat.z_axis[0], mat.z_axis[1], mat.z_axis[2], mat.z_axis[3]},
        {mat.w_axis[0], mat.w_axis[1], mat.w_axis[2], mat.w_axis[3]}};

    return static_cast<Transform3D>(projection);
}

void CarPhysics::simulate(const Ref<CarPhysicsInput>& input)
{
    if (input.is_null())
    {
        UtilityFunctions::printerr("Input is null");
        return;
    }

    physics::State state = physics->simulate(input->as_physics_input());

    Ref<CarPhysicsTransforms> transforms;
    transforms.instantiate();
    transforms->set_body(physics_matrix_to_transform(state.body_transform));
    transforms->set_wheel1(physics_matrix_to_transform(state.wheel_transforms[0]));
    transforms->set_wheel2(physics_matrix_to_transform(state.wheel_transforms[1]));
    transforms->set_wheel3(physics_matrix_to_transform(state.wheel_transforms[2]));
    transforms->set_wheel4(physics_matrix_to_transform(state.wheel_transforms[3]));

    // Node3D* body_node = get_node<Node3D>(body);
    // Node3D* wheel1_node = get_node<Node3D>(wheel1);
    // Node3D* wheel2_node = get_node<Node3D>(wheel2);
    // Node3D* wheel3_node = get_node<Node3D>(wheel3);
    // Node3D* wheel4_node = get_node<Node3D>(wheel4);
    // body_node->set_global_transform(transforms->get_body());
    // wheel1_node->set_global_transform(transforms->get_wheel1());
    // wheel2_node->set_global_transform(transforms->get_wheel2());
    // wheel3_node->set_global_transform(transforms->get_wheel3());
    // wheel4_node->set_global_transform(transforms->get_wheel4());

    // const auto update_nodes_end = time.get_ticks_usec();

    bool input_simulated = !(state.finished && state.last_finished);
    bool just_finished = state.finished && !state.last_finished;

    Ref<CarPhysicsStep> step;
    step.instantiate();

    step->set_step(state.step);
    step->set_input(input);
    step->set_simulated(input_simulated);
    step->set_speed(state.speed);
    step->set_rpm(state.rpm);
    step->set_gear(state.gear);
    step->set_just_finished(just_finished);
    step->set_available_checkpoints(physics->checkpoint_count());
    step->set_collected_checkpoints(physics->collected_checkpoint_count());
    step->set_transforms(transforms);

    emit_signal("simulated", step);
}

PackedByteArray CarPhysics::save_state() const
{
    std::string physics_state = physics->save_state();

    PackedByteArray state;
    CRASH_COND(state.resize(physics_state.size()) != 0);

    size_t i = 0;
    for (const uint8_t element : physics_state)
    {
        state.set(i++, element);
    }

    return state;
}

void CarPhysics::load_state(const PackedByteArray& state)
{
    std::string physics_state;
    physics_state.resize(state.size());

    size_t i = 0;
    for (const char element : state)
    {
        physics_state[i++] = element;
    }

    physics->load_state(physics_state);
}

size_t CarPhysics::checkpoint_count() const
{
    return physics->checkpoint_count();
}

size_t CarPhysics::collected_checkpoint_count() const
{
    return physics->collected_checkpoint_count();
}
