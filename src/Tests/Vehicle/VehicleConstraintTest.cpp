// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Tests/Vehicle/VehicleConstraintTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.h>
#include <Jolt/Physics/Vehicle/WheeledVehicleController.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>
#include <iostream>

JPH_IMPLEMENT_RTTI_VIRTUAL(VehicleConstraintTest)
{
    JPH_ADD_BASE_CLASS(VehicleConstraintTest, VehicleTest)
}

void VehicleConstraintTest::InitStartingTransform(Mat44Arg transform)
{
    starting_transform = transform;
}

VehicleConstraintTest::~VehicleConstraintTest()
{
    mPhysicsSystem->RemoveStepListener(mVehicleConstraint);
}

void VehicleConstraintTest::Initialize()
{
    VehicleTest::Initialize();

    Vec3 scale;
    Mat44 transform = starting_transform.Decompose(scale);
    Vec3 initial_position = transform.GetTranslation();
    Quat initial_rotation = transform.GetRotation().GetQuaternion();

    const float wheel_radius = sWheelRadius;
    const float wheel_width = sWheelWidth;
    // const float half_wheel_width = sWheelWidth / 2.0f;
    const float half_vehicle_length = sVehicleLength / 2.0f;
    const float half_vehicle_width = sVehicleWidth / 2.0f;
    const float half_vehicle_height = sVehicleHeight / 2.0f;
    // Moment of inertia (kg m^2), for a cylinder this would be 0.5 * M * R^2 which is 0.9 for a wheel with a mass of 20 kg and radius 0.3 m
    const float wheel_inertia = 0.5f * (sWheelMass * (wheel_radius * wheel_radius));

    // Create collision testers
    mTesters[0] = new VehicleCollisionTesterRay(Layers::MOVING);
    mTesters[1] = new VehicleCollisionTesterCastSphere(Layers::MOVING, 0.5f * wheel_width);
    mTesters[2] = new VehicleCollisionTesterCastCylinder(Layers::MOVING);

    // Create vehicle body
    RefConst<Shape> car_shape = OffsetCenterOfMassShapeSettings(Vec3(0, -half_vehicle_height, 0), new BoxShape(Vec3(half_vehicle_width, half_vehicle_height, half_vehicle_length))).Create().Get();
    BodyCreationSettings car_body_settings(car_shape, initial_position, initial_rotation, EMotionType::Dynamic, Layers::MOVING);
    car_body_settings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
    car_body_settings.mMassPropertiesOverride.mMass = sBodyMass;
    mCarBody = mBodyInterface->CreateBody(car_body_settings);
    mBodyInterface->AddBody(mCarBody->GetID(), EActivation::Activate);

    // Create vehicle constraint
    VehicleConstraintSettings vehicle;
    vehicle.mDrawConstraintSize = 0.1f;
    vehicle.mMaxPitchRollAngle = sMaxRollAngle;

    // Suspension direction
    Vec3 front_suspension_dir = Vec3(Tan(sFrontSuspensionSidewaysAngle), -1, Tan(sFrontSuspensionForwardAngle)).Normalized();
    Vec3 front_steering_axis = Vec3(-Tan(sFrontKingPinAngle), 1, -Tan(sFrontCasterAngle)).Normalized();
    Vec3 front_wheel_up = Vec3(Sin(sFrontCamber), Cos(sFrontCamber), 0);
    Vec3 front_wheel_forward = Vec3(-Sin(sFrontToe), 0, Cos(sFrontToe));
    Vec3 rear_suspension_dir = Vec3(Tan(sRearSuspensionSidewaysAngle), -1, Tan(sRearSuspensionForwardAngle)).Normalized();
    Vec3 rear_steering_axis = Vec3(-Tan(sRearKingPinAngle), 1, -Tan(sRearCasterAngle)).Normalized();
    Vec3 rear_wheel_up = Vec3(Sin(sRearCamber), Cos(sRearCamber), 0);
    Vec3 rear_wheel_forward = Vec3(-Sin(sRearToe), 0, Cos(sRearToe));
    Vec3 flip_x(-1, 1, 1);

    // Wheels, left front

    const auto w1_pos = Vec3(0.75f, 0.1, 0.95f);
    WheelSettingsWV* w1 = new WheelSettingsWV;
    w1->mPosition = w1_pos;
    w1->mSuspensionDirection = front_suspension_dir;
    w1->mSteeringAxis = front_steering_axis;
    w1->mWheelUp = front_wheel_up;
    w1->mWheelForward = front_wheel_forward;
    w1->mSuspensionMinLength = sFrontSuspensionMinLength;
    w1->mSuspensionMaxLength = sFrontSuspensionMaxLength;
    w1->mSuspensionSpring.mFrequency = sFrontSuspensionFrequency;
    w1->mSuspensionSpring.mDamping = sFrontSuspensionDamping;
    w1->mMaxSteerAngle = sMaxSteeringAngle;
    w1->mMaxHandBrakeTorque = 0.0f;  // Front wheel doesn't have hand brake
    w1->mMaxBrakeTorque = sMaxBrakeTorque;
    w1->mInertia = wheel_inertia;

    // Right front
    const auto w2_pos = Vec3(-0.75f, 0.1f, 0.95f);
    WheelSettingsWV* w2 = new WheelSettingsWV;
    w2->mPosition = w2_pos;
    w2->mSuspensionDirection = flip_x * front_suspension_dir;
    w2->mSteeringAxis = flip_x * front_steering_axis;
    w2->mWheelUp = flip_x * front_wheel_up;
    w2->mWheelForward = flip_x * front_wheel_forward;
    w2->mSuspensionMinLength = sFrontSuspensionMinLength;
    w2->mSuspensionMaxLength = sFrontSuspensionMaxLength;
    w2->mSuspensionSpring.mFrequency = sFrontSuspensionFrequency;
    w2->mSuspensionSpring.mDamping = sFrontSuspensionDamping;
    w2->mMaxSteerAngle = sMaxSteeringAngle;
    w2->mMaxHandBrakeTorque = 0.0f;  // Front wheel doesn't have hand brake
    w2->mMaxBrakeTorque = sMaxBrakeTorque;
    w2->mInertia = wheel_inertia;

    // Left rear
    const auto w3_pos = Vec3(0.75f, 0.1f, -1.32f);
    WheelSettingsWV* w3 = new WheelSettingsWV;
    w3->mPosition = w3_pos;
    w3->mSuspensionDirection = rear_suspension_dir;
    w3->mSteeringAxis = rear_steering_axis;
    w3->mWheelUp = rear_wheel_up;
    w3->mWheelForward = rear_wheel_forward;
    w3->mSuspensionMinLength = sRearSuspensionMinLength;
    w3->mSuspensionMaxLength = sRearSuspensionMaxLength;
    w3->mSuspensionSpring.mFrequency = sRearSuspensionFrequency;
    w3->mSuspensionSpring.mDamping = sRearSuspensionDamping;
    w3->mMaxSteerAngle = 0.0f;
    w3->mMaxHandBrakeTorque = sMaxHandBrakeTorque;
    w3->mMaxBrakeTorque = sMaxBrakeTorque;
    w3->mInertia = wheel_inertia;

    // Right rear
    const auto w4_pos = Vec3(-0.75f, 0.1f, -1.32f);
    WheelSettingsWV* w4 = new WheelSettingsWV;
    w4->mPosition = w4_pos;
    w4->mSuspensionDirection = flip_x * rear_suspension_dir;
    w4->mSteeringAxis = flip_x * rear_steering_axis;
    w4->mWheelUp = flip_x * rear_wheel_up;
    w4->mWheelForward = flip_x * rear_wheel_forward;
    w4->mSuspensionMinLength = sRearSuspensionMinLength;
    w4->mSuspensionMaxLength = sRearSuspensionMaxLength;
    w4->mSuspensionSpring.mFrequency = sRearSuspensionFrequency;
    w4->mSuspensionSpring.mDamping = sRearSuspensionDamping;
    w4->mMaxSteerAngle = 0.0f;
    w4->mMaxHandBrakeTorque = sMaxHandBrakeTorque;
    w4->mMaxBrakeTorque = sMaxBrakeTorque;
    w4->mInertia = wheel_inertia;

    vehicle.mWheels = {w1, w2, w3, w4};

    std::cerr << "w1 " << w1_pos << "\n";
    std::cerr << "w2 " << w2_pos << "\n";
    std::cerr << "w3 " << w3_pos << "\n";
    std::cerr << "w4 " << w4_pos << "\n";

    for (WheelSettings* w : vehicle.mWheels)
    {
        w->mRadius = wheel_radius;
        w->mWidth = wheel_width;
    }

    WheeledVehicleControllerSettings* controller = new WheeledVehicleControllerSettings;
    vehicle.mController = controller;

    // Differential
    controller->mDifferentials.resize(sFourWheelDrive ? 2 : 1);
    controller->mDifferentials[0].mLeftWheel = 0;
    controller->mDifferentials[0].mRightWheel = 1;
    if (sFourWheelDrive)
    {
        controller->mDifferentials[1].mLeftWheel = 2;
        controller->mDifferentials[1].mRightWheel = 3;

        // Split engine torque
        controller->mDifferentials[0].mEngineTorqueRatio = controller->mDifferentials[1].mEngineTorqueRatio = 0.5f;
    }

    // Anti rollbars
    if (sAntiRollbar)
    {
        vehicle.mAntiRollBars.resize(2);
        vehicle.mAntiRollBars[0].mLeftWheel = 0;
        vehicle.mAntiRollBars[0].mRightWheel = 1;
        vehicle.mAntiRollBars[1].mLeftWheel = 2;
        vehicle.mAntiRollBars[1].mRightWheel = 3;
    }

    mVehicleConstraint = new VehicleConstraint(*mCarBody, vehicle);
    mVehicleConstraint->SetMaxPitchRollAngle(JPH_PI);
    mPhysicsSystem->AddConstraint(mVehicleConstraint);
    mPhysicsSystem->AddStepListener(mVehicleConstraint);

    mVehicleConstraint->SaveState(initial_state);
}

void VehicleConstraintTest::PrePhysicsUpdate(const PreUpdateParams& inParams)
{
    VehicleTest::PrePhysicsUpdate(inParams);

    // Determine acceleration and brake
    float forward = 0.0f, right = 0.0f, brake = 0.0f, hand_brake = 0.0f;
    if (inParams.mKeyboard->IsKeyPressed(DIK_UP))
        forward = 1.0f;
    else if (inParams.mKeyboard->IsKeyPressed(DIK_DOWN))
        forward = -1.0f;

    // Check if we're reversing direction
    if (mPreviousForward * forward < 0.0f)
    {
        // Get vehicle velocity in local space to the body of the vehicle
        float velocity = (mCarBody->GetRotation().Conjugated() * mCarBody->GetLinearVelocity()).GetZ();
        if ((forward > 0.0f && velocity < -0.1f) || (forward < 0.0f && velocity > 0.1f))
        {
            // Brake while we've not stopped yet
            forward = 0.0f;
            brake = 1.0f;
        }
        else
        {
            // When we've come to a stop, accept the new direction
            mPreviousForward = forward;
        }
    }

    // Hand brake will cancel gas pedal
    if (inParams.mKeyboard->IsKeyPressed(DIK_Z))
    {
        forward = 0.0f;
        hand_brake = 1.0f;
    }

    // Steering
    if (inParams.mKeyboard->IsKeyPressed(DIK_LEFT))
        right = -1.0f;
    else if (inParams.mKeyboard->IsKeyPressed(DIK_RIGHT))
        right = 1.0f;

    // On user input, assure that the car is active
    if (right != 0.0f || forward != 0.0f || brake != 0.0f || hand_brake != 0.0f)
        mBodyInterface->ActivateBody(mCarBody->GetID());

    WheeledVehicleController* controller = static_cast<WheeledVehicleController*>(mVehicleConstraint->GetController());

    // Update vehicle statistics
    controller->GetEngine().mMaxTorque = sMaxEngineTorque;
    auto& transmission = controller->GetTransmission();
    transmission.mSwitchTime = sSwitchTime;
    transmission.mClutchReleaseTime = sClutchReleaseTime;
    transmission.mSwitchLatency = sSwitchLatency;
    transmission.mShiftUpRPM = sShiftUpRPM;
    transmission.mShiftDownRPM = sShiftDownRPM;
    transmission.mClutchStrength = sClutchStrength;

    // Set slip ratios to the same for everything
    float limited_slip_ratio = sLimitedSlipDifferentials ? 1.4f : FLT_MAX;
    controller->SetDifferentialLimitedSlipRatio(limited_slip_ratio);
    for (VehicleDifferentialSettings& d : controller->GetDifferentials())
        d.mLimitedSlipRatio = limited_slip_ratio;

    // Pass the input on to the constraint
    controller->SetDriverInput(forward, right, brake, hand_brake);

    // Set the collision tester
    mVehicleConstraint->SetVehicleCollisionTester(mTesters[sCollisionMode]);
}

void VehicleConstraintTest::ResetTo(Vec3Arg translation, QuatArg rotation)
{
    initial_state.Rewind();
    mVehicleConstraint->RestoreState(initial_state);
    mBodyInterface->SetPositionRotationAndVelocity(mCarBody->GetID(), translation, rotation, Vec3::sZero(), Vec3::sZero());
}

void VehicleConstraintTest::SaveState(StateRecorder& inStream) const
{
    VehicleTest::SaveState(inStream);

    inStream.Write(mPreviousForward);
}

void VehicleConstraintTest::RestoreState(StateRecorder& inStream)
{
    VehicleTest::RestoreState(inStream);

    inStream.Read(mPreviousForward);
}

RMat44 VehicleConstraintTest::GetWheelWorldTransform(uint inWheelIndex) const
{
    return mVehicleConstraint->GetWheelWorldTransform(inWheelIndex, Vec3::sAxisY(), Vec3::sAxisX());
}

VehicleConstraintTest::VehicleInfo VehicleConstraintTest::GetVehicleInfo() const
{
    WheeledVehicleController* controller = static_cast<WheeledVehicleController*>(mVehicleConstraint->GetController());
    const auto speed = mCarBody->GetLinearVelocity().Dot(mCarBody->GetRotation().RotateAxisZ());
    const auto rpm = controller->GetEngine().GetCurrentRPM();
    const auto gear = controller->GetTransmission().GetCurrentGear();
    return {
        speed,
        rpm,
        gear,
    };
}
