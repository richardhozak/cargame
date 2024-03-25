// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Vehicle/VehicleTest.h>
#include <Jolt/Physics/Constraints/SixDOFConstraint.h>

#include <array>

// This test shows how a vehicle could be made with the SixDOF constraint.
class VehicleSixDOFTest : public VehicleTest
{
public:
    JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, VehicleSixDOFTest)

    // See: Test
    virtual void Initialize() override;
    virtual void PrePhysicsUpdate(const PreUpdateParams& inParams) override;

    Body& GetVehicleBody()
    {
        return *mCarBody;
    }

    std::array<Body*, 4> GetWheelBodies()
    {
        return wheel_bodies;
    }

private:
    static constexpr float cMaxSteeringAngle = DegreesToRadians(30);

    using EAxis = SixDOFConstraintSettings::EAxis;

    enum class EWheel : int
    {
        LeftFront,
        RightFront,
        LeftRear,
        RightRear,
        Num,
    };

    static inline bool sIsFrontWheel(EWheel inWheel)
    {
        return inWheel == EWheel::LeftFront || inWheel == EWheel::RightFront;
    }
    static inline bool sIsLeftWheel(EWheel inWheel)
    {
        return inWheel == EWheel::LeftFront || inWheel == EWheel::LeftRear;
    }

    Body* mCarBody;
    Ref<SixDOFConstraint> mWheels[int(EWheel::Num)];
    std::array<Body*, 4> wheel_bodies;
};
