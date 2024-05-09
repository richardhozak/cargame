
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

// The Jolt headers don't include Jolt.h. Always include Jolt.h before including any other Jolt header.
// You can use Jolt.h in your precompiled header to speed up compilation.
#include <Jolt/Jolt.h>

// Jolt includes
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>

#include "Input/Keyboard.h"
#include "Jolt/Math/MathTypes.h"
#include "Jolt/Math/Quat.h"
#include "Jolt/Math/Real.h"
#include "Jolt/Math/Vec3.h"
#include "Jolt/Math/Vec4.h"
#include "Jolt/Physics/Body/BodyID.h"
#include "Jolt/Physics/Body/BodyInterface.h"
#include "Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h"
#include "Jolt/Physics/Collision/Shape/StaticCompoundShape.h"
#include "Jolt/Physics/EActivation.h"
#include "Jolt/Physics/StateRecorder.h"
#include "Jolt/Physics/StateRecorderImpl.h"
#include "Layers.h"
#include "Tests/Vehicle/VehicleConstraintTest.h"
// #include "Tests/Vehicle/VehicleSixDOFTest.h"

// STL includes
#include <iostream>
#include <cstdarg>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <array>
#include <set>
#include <string>

#include "physics.h"

// Disable common warnings triggered by Jolt, you can use JPH_SUPPRESS_WARNING_PUSH / JPH_SUPPRESS_WARNING_POP to store and restore the warning state
JPH_SUPPRESS_WARNINGS

// All Jolt symbols are in the JPH namespace
using namespace JPH;

// If you want your code to compile using single or double precision write 0.0_r to get a Real value that compiles to double or float depending if JPH_DOUBLE_PRECISION is set or not.
using namespace JPH::literals;

// We're also using STL classes in this example
using namespace std;

// Callback for traces, connect this to your own trace function if you have one
static void TraceImpl(const char* inFMT, ...)
{
    // Format the message
    va_list list;
    va_start(list, inFMT);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), inFMT, list);
    va_end(list);

    // Print to the TTY
    cout << buffer << endl;
}

#ifdef JPH_ENABLE_ASSERTS

// Callback for asserts, connect this to your own assert handler if you have one
static bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, uint inLine)
{
    // Print to the TTY
    cout << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr ? inMessage : "") << endl;

    // Breakpoint
    return true;
};

#endif  // JPH_ENABLE_ASSERTS

struct Initialization
{
    Initialization()
    {
        // Register allocation hook
        RegisterDefaultAllocator();

        // Install callbacks
        Trace = TraceImpl;
        JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)

        // Create a factory
        Factory::sInstance = new Factory();

        // Register all Jolt physics types
        RegisterTypes();
    }

    ~Initialization()
    {
        // Unregisters all types with the factory and cleans up the default material
        UnregisterTypes();

        // Destroy the factory
        delete Factory::sInstance;
        Factory::sInstance = nullptr;
    }
};

Vec3 physics_vec3(const physics::Vector3& vec)
{
    return Vec3(vec.x, vec.y, vec.z);
}

Vec4 physics_vec4(const physics::Vector4& vec)
{
    return Vec4(vec.x, vec.y, vec.z, vec.w);
}

Mat44 mat_to_mat(const physics::Matrix4& mat)
{
    return Mat44(
        Vec4(mat.x_axis[0], mat.x_axis[1], mat.x_axis[2], mat.x_axis[3]),
        Vec4(mat.y_axis[0], mat.y_axis[1], mat.y_axis[2], mat.y_axis[3]),
        Vec4(mat.z_axis[0], mat.z_axis[1], mat.z_axis[2], mat.z_axis[3]),
        Vec4(mat.w_axis[0], mat.w_axis[1], mat.w_axis[2], mat.w_axis[3]));
}

physics::Matrix4 transform_to_transform(const Mat44& transform)
{
    auto col1 = transform.GetColumn4(0);
    auto col2 = transform.GetColumn4(1);
    auto col3 = transform.GetColumn4(2);
    auto col4 = transform.GetColumn4(3);

    return physics::Matrix4{
        {col1.GetX(), col1.GetY(), col1.GetZ(), col1.GetW()},
        {col2.GetX(), col2.GetY(), col2.GetZ(), col2.GetW()},
        {col3.GetX(), col3.GetY(), col3.GetZ(), col3.GetW()},
        {col4.GetX(), col4.GetY(), col4.GetZ(), col4.GetW()},
    };
}

enum SensorType : uint8_t
{
    Finish = 1,
    Checkpoint = 2,
};

RMat44 matrix_from_trs(Vec3Arg translation, QuatArg rotation, Vec3Arg scale)
{
    auto mat_scale = Mat44::sScale(scale);
    auto mat_rot_pos = Mat44::sRotationTranslation(rotation, translation);
    auto mat_local = mat_rot_pos * mat_scale;
    return mat_local;
}

std::string mesh_kind_name(physics::MeshKind kind)
{
    switch (kind)
    {
        case physics::MeshKind::Start:
            return "Start";
        case physics::MeshKind::Road:
            return "Road";
        case physics::MeshKind::Finish:
            return "Finish";
        case physics::MeshKind::Checkpoint:
            return "Checkpoint";
        default:
            return "";
    }
}

namespace physics {
// Toy implementation of an in-memory blobstore.
//
// In reality the implementation of Physics could be a large complex C++
// library.
class Physics::impl : public ContactListener
{
    friend Physics;

    // This is the max amount of rigid bodies that you can add to the physics system. If you try to add more you'll get an error.
    // Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
    const uint cMaxBodies = 1024;

    // This determines how many mutexes to allocate to protect rigid bodies from concurrent access. Set it to 0 for the default settings.
    const uint cNumBodyMutexes = 0;

    // This is the max amount of body pairs that can be queued at any time (the broad phase will detect overlapping
    // body pairs based on their bounding boxes and will insert them into a queue for the narrowphase). If you make this buffer
    // too small the queue will fill up and the broad phase jobs will start to do narrow phase work. This is slightly less efficient.
    // Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
    const uint cMaxBodyPairs = 1024;

    // This is the maximum size of the contact constraint buffer. If more contacts (collisions between bodies) are detected than this
    // number then these contacts will be ignored and bodies will start interpenetrating / fall through the world.
    // Note: This value is low because this is a simple test. For a real project use something in the order of 10240.
    const uint cMaxContactConstraints = 1024;

    // We need a temp allocator for temporary allocations during the physics update. We're
    // pre-allocating 10 MB to avoid having to do allocations during the physics update.
    // B.t.w. 10 MB is way too much for this example but it is a typical value you can use.
    // If you don't want to pre-allocate you can also use TempAllocatorMalloc to fall back to
    // malloc / free.
    TempAllocatorImpl temp_allocator;

    // We need a job system that will execute physics jobs on multiple threads. Typically
    // you would implement the JobSystem interface yourself and let Jolt Physics run on top
    // of your own job scheduler. JobSystemThreadPool is an example implementation.
    JobSystemThreadPool job_system;

    // Create mapping table from object layer to broadphase layer
    // Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
    BPLayerInterfaceImpl broad_phase_layer_interface;

    // Create class that filters object vs broadphase layers
    // Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
    ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;

    // Create class that filters object vs object layers
    // Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
    ObjectLayerPairFilterImpl object_vs_object_layer_filter;

    // Now we can create the actual physics system.
    PhysicsSystem physics_system;

    int64_t step = 0;

    std::unique_ptr<VehicleConstraintTest> vehicle_test;
    // std::unique_ptr<VehicleSixDOFTest> vehicle_test;
    std::string start_state;

    Mat44 starting_transform = Mat44::sIdentity();

    std::map<BodyID, Mat44> available_checkpoints;
    std::set<BodyID> collected_checkpoints;
    bool last_finished = false;
    bool finished = false;
    Mutex checkpoint_mutex;
    std::optional<BodyID> last_collected_checkpoint;

    impl(const Track& track)
      : temp_allocator(10 * 1024 * 1024)
      , job_system(cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1)
    {
        physics_system.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, broad_phase_layer_interface, object_vs_broadphase_layer_filter, object_vs_object_layer_filter);
        physics_system.SetContactListener(this);

        load_track(track);

        // start with a countdown
        step = -3 * 100;

        vehicle_test = setup_vehicle();

        physics_system.OptimizeBroadPhase();
    }

    struct ContactHelper
    {
        static std::optional<ContactHelper> create(const Body& inBody1, const Body& inBody2, const BodyID player_id)
        {
            if (inBody1.GetID() == player_id)
            {
                return ContactHelper(inBody1, inBody2);
            }
            else if (inBody2.GetID() == player_id)
            {
                return ContactHelper(inBody2, inBody1);
            }
            else
            {
                return {};
            }
        }

        const Body& player;
        const Body& other;

    private:
        ContactHelper(const Body& player, const Body& other)
          : player(player)
          , other(other)
        {}
    };

    virtual void OnContactAdded(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings) override
    {
        // note that this is called from multiple thread at once, we need to lock any resources we want to write from here

        const auto contact = ContactHelper::create(inBody1, inBody2, vehicle_test->GetVehicleBody().GetID());
        if (!contact)
        {
            return;
        }

        if (!contact->other.IsSensor())
        {
            return;
        }

        lock_guard lock(checkpoint_mutex);

        if (contact->other.GetUserData() == static_cast<uint64_t>(SensorType::Finish))
        {
            if (available_checkpoints.size() - collected_checkpoints.size() == 0)
            {
                finished = true;
            }
        }
        else if (contact->other.GetUserData() == static_cast<uint64_t>(SensorType::Checkpoint))
        {
            collected_checkpoints.insert(contact->other.GetID());
            last_collected_checkpoint = contact->other.GetID();
        }
    }

    void load_track(const Track& track)
    {
        if (track.meshes.empty())
        {
            return;
        }

        const Vec3 track_scale = physics_vec3(track.scale);

        BodyInterface& body_interface = physics_system.GetBodyInterface();

        Ref<StaticCompoundShapeSettings> compound_shape = new StaticCompoundShapeSettings;

        for (const auto& mesh : track.meshes)
        {
            const auto translation = physics_vec3(mesh.position);
            const auto rotation = Quat(physics_vec4(mesh.rotation));
            const auto scale = physics_vec3(mesh.scale);

            VertexList vertex_list;
            for (const auto& vertex : mesh.vertices)
            {
                vertex_list.emplace_back(vertex[0], vertex[1], vertex[2]);
            }

            IndexedTriangleList indexed_triangle_list;
            for (size_t i = 0; i < mesh.indices.size(); i += 3)
            {
                indexed_triangle_list.emplace_back(IndexedTriangle(mesh.indices[i], mesh.indices[i + 2], mesh.indices[i + 1]));
            }

            RefConst<MeshShapeSettings> mesh_shape = new MeshShapeSettings(vertex_list, indexed_triangle_list);
            RefConst<ScaledShapeSettings> scaled_shape = new ScaledShapeSettings(mesh_shape, scale);

            switch (mesh.kind)
            {
                case MeshKind::Start:
                {
                    // do not add this to physics, just use it as a starting position for car
                    starting_transform = matrix_from_trs(translation, rotation, scale).PostScaled(track_scale);
                    continue;
                }
                case MeshKind::Finish:
                {
                    RefConst<RotatedTranslatedShapeSettings> rt_finish_shape = new RotatedTranslatedShapeSettings(translation, rotation, scaled_shape);
                    RefConst<ScaledShapeSettings> s_finish_shape = new ScaledShapeSettings(rt_finish_shape, track_scale);
                    Body& finish = *body_interface.CreateBody(BodyCreationSettings(s_finish_shape, RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::SENSOR));
                    finish.SetIsSensor(true);
                    finish.SetUserData(static_cast<uint64_t>(SensorType::Finish));
                    body_interface.AddBody(finish.GetID(), EActivation::DontActivate);
                    continue;
                }
                case MeshKind::Road:
                {
                    compound_shape->AddShape(translation, rotation, scaled_shape);
                    break;
                }
                case MeshKind::Checkpoint:
                {
                    RefConst<RotatedTranslatedShapeSettings> rt_cp_shape = new RotatedTranslatedShapeSettings(translation, rotation, scaled_shape);
                    RefConst<ScaledShapeSettings> s_cp_shape = new ScaledShapeSettings(rt_cp_shape, track_scale);
                    Body& cp = *body_interface.CreateBody(BodyCreationSettings(s_cp_shape, RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::SENSOR));
                    cp.SetIsSensor(true);
                    cp.SetUserData(static_cast<uint64_t>(SensorType::Checkpoint));
                    body_interface.AddBody(cp.GetID(), EActivation::DontActivate);
                    available_checkpoints.insert({cp.GetID(), matrix_from_trs(translation, rotation, scale).PostScaled(track_scale)});
                    continue;
                }
            }
        }

        RefConst<ScaledShapeSettings> scaled_shape = new ScaledShapeSettings(compound_shape, track_scale);
        Body& floor = *body_interface.CreateBody(BodyCreationSettings(scaled_shape, RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
        body_interface.AddBody(floor.GetID(), EActivation::DontActivate);
        floor.SetFriction(1.0);
    }

    std::unique_ptr<VehicleConstraintTest> setup_vehicle()
    {
        auto test = std::make_unique<VehicleConstraintTest>();

        test->SetPhysicsSystem(&physics_system);
        test->SetJobSystem(&job_system);
        test->SetTempAllocator(&temp_allocator);

        // move the position a bit up so car does not start inside a track
        test->InitStartingTransform(starting_transform.PostTranslated(Vec3(0.0, 1.0, 0.0)));

        test->Initialize();

        return test;
    }

    // std::unique_ptr<VehicleSixDOFTest> setup_vehicle()
    // {
    //     (void)configuration;

    //     auto test = std::make_unique<VehicleSixDOFTest>();

    //     test->SetPhysicsSystem(&physics_system);
    //     test->SetJobSystem(&job_system);
    //     test->SetTempAllocator(&temp_allocator);
    //     test->Initialize();

    //     return test;
    // }

    State current_simulation_state()
    {
        State state;

        for (uint w = 0; w < 4; w++)
        {
            state.wheel_transforms[w] = transform_to_transform(vehicle_test->GetWheelWorldTransform(w));
        }

        // {
        //     const auto wheels = vehicle_test->GetWheelBodies();

        //     int i = 0;
        //     for (const auto wheel : wheels)
        //     {
        //         const auto transform = wheel->GetWorldTransform();
        //         state.wheel_transforms[i] = transform_to_transform(transform);

        //         i++;
        //     }
        // }

        {
            const auto& body = vehicle_test->GetVehicleBody();
            const auto transform = body.GetWorldTransform();
            state.body_transform = transform_to_transform(transform);
        }

        {
            const auto info = vehicle_test->GetVehicleInfo();
            state.speed = info.speed;
            state.rpm = info.rpm;
            state.gear = info.gear;
        }

        state.step = step;
        state.last_finished = last_finished;
        state.finished = finished;

        state.collected_checkpoints = collected_checkpoints.size();

        return state;
    }
};

Physics::Physics(const Track& track)
  : impl(new class Physics::impl(track))
{
    impl->start_state = save_state();
}

std::string Physics::save_state() const
{
    StateRecorderImpl recorder;

    impl->physics_system.SaveState(recorder);

    impl->vehicle_test->SaveState(recorder);

    recorder.Write(impl->last_finished);
    recorder.Write(impl->finished);

    recorder.Write(impl->step);

    recorder.Write(static_cast<uint32_t>(impl->collected_checkpoints.size()));
    for (const BodyID id : impl->collected_checkpoints)
    {
        recorder.Write(id.GetIndexAndSequenceNumber());
    }

    recorder.Write(impl->last_collected_checkpoint.has_value());
    if (impl->last_collected_checkpoint.has_value())
    {
        recorder.Write(impl->last_collected_checkpoint->GetIndexAndSequenceNumber());
    }

    return recorder.GetData();
}

void Physics::load_state(const std::string& bytes)
{
    StateRecorderImpl recorder;
    recorder.WriteBytes(bytes.data(), bytes.size());

    impl->physics_system.RestoreState(recorder);

    impl->vehicle_test->RestoreState(recorder);

    recorder.Read(impl->last_finished);
    recorder.Read(impl->finished);

    recorder.Read(impl->step);

    uint32_t collected_checkpoints_count = 0;
    recorder.Read(collected_checkpoints_count);

    std::set<BodyID> collected_checkpoints;
    for (uint32_t i = 0; i < collected_checkpoints_count; i++)
    {
        uint32_t body_id = 0;
        recorder.Read(body_id);

        collected_checkpoints.insert(BodyID(body_id));
    }

    impl->collected_checkpoints = collected_checkpoints;

    std::optional<BodyID> last_collected_checkpoint = std::nullopt;
    bool has_last_collected_checkpoint = false;
    recorder.Read(has_last_collected_checkpoint);

    if (has_last_collected_checkpoint)
    {
        uint32_t body_id = 0;
        recorder.Read(body_id);
        last_collected_checkpoint = BodyID(body_id);
    }

    impl->last_collected_checkpoint = last_collected_checkpoint;
}

State Physics::simulate(const Input& input)
{
    // We simulate the physics world in discrete time steps. 100 Hz is a good rate to update the physics system.
    const float cDeltaTime = 1.0f / 100.0f;

    impl->last_finished = impl->finished;

    if (input.restart)
    {
        load_state(impl->start_state);
        return impl->current_simulation_state();
    }
    else
    {
        if (impl->step < 0)
        {
            impl->step += 1;
            return impl->current_simulation_state();
        }

        std::set<DIK> pressed;

        if (!impl->finished)
        {
            if (input.up)
                pressed.insert(DIK::DIK_UP);
            if (input.down)
                pressed.insert(DIK::DIK_DOWN);
            if (input.left)
                pressed.insert(DIK::DIK_LEFT);
            if (input.right)
                pressed.insert(DIK::DIK_RIGHT);
            if (input.brake)
                pressed.insert(DIK::DIK_Z);
        }

        if (input.respawn)
        {
            if (impl->last_collected_checkpoint.has_value())
            {
                const Mat44 checkpoint_mat = impl->available_checkpoints.at(*impl->last_collected_checkpoint).PostTranslated(Vec3(0.0, 1.0, 0.0));
                Vec3 scale;
                const Mat44 decomposed = checkpoint_mat.Decompose(scale);
                const Vec3 translation = decomposed.GetTranslation();
                const Quat rotation = decomposed.GetRotation().GetQuaternion();

                impl->vehicle_test->ResetTo(translation, rotation);
            }
        }

        Keyboard kbd(pressed);

        VehicleTest::PreUpdateParams params;
        params.mDeltaTime = cDeltaTime;
        params.mKeyboard = &kbd;
        impl->vehicle_test->PrePhysicsUpdate(params);

        // If you take larger steps than 1 / 100th of a second you need to do multiple collision steps in order to keep the simulation stable. Do 1 collision step per 1 / 100th of a second (round up).
        const int cCollisionSteps = 1;

        if (!impl->finished)
        {
            // Next step
            impl->step += 1;
        }

        // Step the world
        impl->physics_system.Update(cDeltaTime, cCollisionSteps, &impl->temp_allocator, &impl->job_system);
        return impl->current_simulation_state();
    }
}

size_t Physics::checkpoint_count() const
{
    return impl->available_checkpoints.size();
}

size_t Physics::collected_checkpoint_count() const
{
    return impl->collected_checkpoints.size();
}

std::unique_ptr<Physics> new_physics(const Track& track)
{
    cerr << "Creating physics with " << track.meshes.size() << " meshes\n";
    static Initialization initialization;
    return std::make_unique<Physics>(track);
}

}  // namespace physics
