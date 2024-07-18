#include "PhysXD6Joint.h"
#include "PhysicsPublic.h"
#include "PhysXPublic.h"


#if WITH_EDITOR
#include "UnrealEd.h"
#include "PhysXD6Joint_Visualizer.h"
static bool gRegisteredVisualizer = false;
#endif

UPhysXD6Joint::UPhysXD6Joint() : UPhysXJointBase()
{
#if WITH_EDITOR
    if (!gRegisteredVisualizer && GUnrealEd)
    {
        gRegisteredVisualizer = true;
        TSharedPtr<FComponentVisualizer> visualizer = MakeShareable(new PhysXD6Joint_Visualizer());
        GUnrealEd->RegisterComponentVisualizer(UPhysXD6Joint::StaticClass()->GetFName(), visualizer);
        visualizer->OnRegister();
    }
#endif

    MotionX = EPxD6Motion::Locked;
    MotionY = EPxD6Motion::Locked;
    MotionZ = EPxD6Motion::Locked;
    MotionTwist = EPxD6Motion::Locked;
    MotionSwing1 = EPxD6Motion::Locked;
    MotionSwing2 = EPxD6Motion::Locked;

    DistanceLimit = 0.0f;

    TwistLimit = FVector2D::ZeroVector;
    SwingLimitY = 0.0f;
    SwingLimitZ = 0.0f;

    DrivePosition = FTransform::Identity;
    DriveLinearVelocity = FVector::ZeroVector;
    DriveAngularVelocity = FVector::ZeroVector;

    // Initialize visualization properties
    bVisualizeJoint = true;
    VisualizationScale = 100.0f;
    LinearLimitColor = FColor::Green;
    SwingLimitColor = FColor::Yellow;
    TwistLimitColor = FColor::Red;
    DriveColor = FColor::Blue;
}

void UPhysXD6Joint::CreateJoint(physx::PxPhysics* PxPhysics)
{
    if (!Actor0 || !Actor1)
    {
        UE_LOG(LogPhysics, Warning, TEXT("Cannot create D6Joint: Invalid actors"));
        return;
    }
    
    UPhysXRigidBody* uRigidBody0 = Actor0->FindComponentByClass<UPhysXRigidBody>();
    UPhysXRigidBody* uRigidBody1 = Actor1->FindComponentByClass<UPhysXRigidBody>();

    if (!uRigidBody0 || !uRigidBody1)
    {
        UE_LOG(LogPhysics, Warning, TEXT("Cannot create D6Joint: Invalid UPhysXRigidBody AActors"));
        return;
    }

    PxRigidActor* PxActor0 = uRigidBody0->GetPhysXRigidActor();
    PxRigidActor* PxActor1 = uRigidBody1->GetPhysXRigidActor();

    if (!PxActor0 || !PxActor1)
    {
        UE_LOG(LogPhysics, Warning, TEXT("Cannot create D6Joint: Invalid PhysX actors"));
        return;
    }

    PxTransform PxLocalPose0 = uRigidBody0->MakePxTransform();
    PxTransform PxLocalPose1 = uRigidBody1->MakePxTransform();

    D6Joint = PxD6JointCreate(*PxPhysics, PxActor0, PxLocalPose0, PxActor1, PxLocalPose1);

    if (!D6Joint)
    {
        UE_LOG(LogPhysics, Warning, TEXT("Failed to create D6Joint"));
        return;
    }

    // Set up joint properties
    SetMotion(EPxD6Axis::X, MotionX);
    SetMotion(EPxD6Axis::Y, MotionY);
    SetMotion(EPxD6Axis::Z, MotionZ);
    SetMotion(EPxD6Axis::Twist, MotionTwist);
    SetMotion(EPxD6Axis::Swing1, MotionSwing1);
    SetMotion(EPxD6Axis::Swing2, MotionSwing2);

    SetDistanceLimit(DistanceLimit);

    SetTwistLimit(TwistLimit);
    SetSwingLimit(SwingLimitY, SwingLimitZ);

    SetDrive(EPxD6Drive::X, DriveX);
    SetDrive(EPxD6Drive::Y, DriveY);
    SetDrive(EPxD6Drive::Z, DriveZ);
    SetDrive(EPxD6Drive::Swing, DriveSwing);
    SetDrive(EPxD6Drive::Twist, DriveTwist);
    SetDrive(EPxD6Drive::Slerp, DriveSlerp);

    SetDrivePosition(DrivePosition);
    SetDriveVelocity(DriveLinearVelocity, DriveAngularVelocity);

    D6Joint->setBreakForce(BreakForce, BreakTorque);

    PxConstraintFlags Flags;
    Flags |= bBroken ? PxConstraintFlag::eBROKEN : PxConstraintFlag::Enum(0);
    Flags |= bCollisionEnabled ? PxConstraintFlag::eCOLLISION_ENABLED : PxConstraintFlag::Enum(0);
    Flags |= bVisualization ? PxConstraintFlag::eVISUALIZATION : PxConstraintFlag::Enum(0);
    Flags |= bDriveLimitsAreForces ? PxConstraintFlag::eDRIVE_LIMITS_ARE_FORCES : PxConstraintFlag::Enum(0);
    Flags |= bImprovedSlerp ? PxConstraintFlag::eIMPROVED_SLERP : PxConstraintFlag::Enum(0);
    Flags |= bDisablePreprocessing ? PxConstraintFlag::eDISABLE_PREPROCESSING : PxConstraintFlag::Enum(0);
    Flags |= bEnableExtendedLimits ? PxConstraintFlag::eENABLE_EXTENDED_LIMITS : PxConstraintFlag::Enum(0);
    Flags |= bGpuCompatible ? PxConstraintFlag::eGPU_COMPATIBLE : PxConstraintFlag::Enum(0);
    Flags |= bAlwaysUpdate ? PxConstraintFlag::eALWAYS_UPDATE : PxConstraintFlag::Enum(0);
    Flags |= bDisableConstraint ? PxConstraintFlag::eDISABLE_CONSTRAINT : PxConstraintFlag::Enum(0);

    D6Joint->setConstraintFlags(Flags);

    D6Joint->setInvMassScale0(InvMassScale0);
    D6Joint->setInvMassScale1(InvMassScale1);
    D6Joint->setInvInertiaScale0(InvInertiaScale0);
    D6Joint->setInvInertiaScale1(InvInertiaScale1);

}

void UPhysXD6Joint::ReleaseJoint()
{
    if (D6Joint)
    {
        D6Joint->release();
        D6Joint = nullptr;
    }
}

float UPhysXD6Joint::GetTwistAngle() const
{
    return D6Joint ? D6Joint->getTwistAngle() : 0.0f;
}

float UPhysXD6Joint::GetSwingYAngle() const
{
    return D6Joint ? D6Joint->getSwingYAngle() : 0.0f;
}

float UPhysXD6Joint::GetSwingZAngle() const
{
    return D6Joint ? D6Joint->getSwingZAngle() : 0.0f;
}

void UPhysXD6Joint::SetMotion(EPxD6Axis Axis, EPxD6Motion Motion)
{
    if (D6Joint)
    {
        PxD6Axis::Enum PxAxis = static_cast<PxD6Axis::Enum>(Axis);
        PxD6Motion::Enum PxMotion = static_cast<PxD6Motion::Enum>(Motion);
        D6Joint->setMotion(PxAxis, PxMotion);
    }
}

void UPhysXD6Joint::SetDistanceLimit(float Limit)
{
    if (D6Joint)
    {
        PxJointLinearLimit PxLimit(Limit);
        D6Joint->setDistanceLimit(PxLimit);
    }
}

void UPhysXD6Joint::SetLinearLimit(EPxD6Axis Axis, const FVector2D& Limit)
{
    if (D6Joint)
    {
        PxD6Axis::Enum PxAxis = static_cast<PxD6Axis::Enum>(Axis);
        PxJointLinearLimitPair PxLimit(PxTolerancesScale(), Limit.X, Limit.Y);
        D6Joint->setLinearLimit(PxAxis, PxLimit);
    }
}

void UPhysXD6Joint::SetTwistLimit(const FVector2D& Limit)
{
    if (D6Joint)
    {
        PxJointAngularLimitPair PxLimit(Limit.X, Limit.Y);
        D6Joint->setTwistLimit(PxLimit);
    }
}

void UPhysXD6Joint::SetSwingLimit(float YLimit, float ZLimit)
{
    if (D6Joint)
    {
        PxJointLimitCone PxLimit(YLimit, ZLimit);
        D6Joint->setSwingLimit(PxLimit);
    }
}

void UPhysXD6Joint::SetDrive(EPxD6Drive DriveType, const FPxD6JointDrive& Drive)
{
    if (D6Joint)
    {
        PxD6JointDrive PxDrive(Drive.Stiffness, Drive.Damping, Drive.ForceLimit, Drive.bAcceleration);
        D6Joint->setDrive(static_cast<PxD6Drive::Enum>(DriveType), PxDrive);
    }
}

void UPhysXD6Joint::SetDrivePosition(const FTransform& Pose, bool bAutowake)
{
    if (D6Joint)
    {
        D6Joint->setDrivePosition(UPhysXRigidBody::MakePxTransform(Pose), bAutowake);
    }
}

void UPhysXD6Joint::SetDriveVelocity(const FVector& Linear, const FVector& Angular, bool bAutowake)
{
    if (D6Joint)
    {
        PxVec3 linear(Linear.X, Linear.Y, Linear.Z);
        PxVec3 angluar(Angular.X, Angular.Y, Angular.Z);
        D6Joint->setDriveVelocity(angluar, angluar, bAutowake);
    }
}

