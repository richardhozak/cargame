// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <TestFramework.h>

#include <Jolt/Physics/PhysicsSystem.h>
#include <Input/Keyboard.h>
#include <Jolt/Core/RTTI.h>

class DebugUI;
class UIElement;
namespace JPH {
class StateRecorder;
class JobSystem;
class ContactListener;
class BodyInterface;
}  // namespace JPH

class VehicleTest
{
public:
    JPH_DECLARE_RTTI_VIRTUAL_BASE(JPH_NO_EXPORT, VehicleTest)

    // Destructor
    virtual ~VehicleTest() = default;

    // Set the physics system
    virtual void SetPhysicsSystem(PhysicsSystem* inPhysicsSystem)
    {
        mPhysicsSystem = inPhysicsSystem;
        mBodyInterface = &inPhysicsSystem->GetBodyInterface();
    }

    // Set the job system
    void SetJobSystem(JobSystem* inJobSystem)
    {
        mJobSystem = inJobSystem;
    }

    // Set the temp allocator
    void SetTempAllocator(TempAllocator* inTempAllocator)
    {
        mTempAllocator = inTempAllocator;
    }

    // Initialize the test
    virtual void Initialize() {}

    // Number used to scale the terrain and camera movement to the scene
    virtual float GetWorldScale() const
    {
        return 1.0f;
    }

    // If this test implements a contact listener, it should be returned here
    virtual ContactListener* GetContactListener()
    {
        return nullptr;
    }

    class PreUpdateParams
    {
    public:
        float mDeltaTime;
        Keyboard* mKeyboard;
    };

    // Update the test, called before the physics update
    virtual void PrePhysicsUpdate(const PreUpdateParams& inParams) {}

    // Saving / restoring state for replay
    virtual void SaveState(StateRecorder& inStream) const {}
    virtual void RestoreState(StateRecorder& inStream) {}

    struct VehicleInfo
    {
        float speed;
        float rpm;
        int32_t gear;
    };

    virtual VehicleInfo GetVehicleInfo() const
    {
        return {};
    };

protected:
    JobSystem* mJobSystem = nullptr;
    PhysicsSystem* mPhysicsSystem = nullptr;
    BodyInterface* mBodyInterface = nullptr;
    TempAllocator* mTempAllocator = nullptr;
};
