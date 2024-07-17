#include "PhysXJointBase.h"

UPhysXJointBase::UPhysXJointBase()
{
    PrimaryComponentTick.bCanEverTick = false;

    Actor0 = nullptr;
    Actor1 = nullptr;
    LocalPose0 = FTransform::Identity;
    LocalPose1 = FTransform::Identity;
    BreakForce = 0.0f;
    BreakTorque = 0.0f;
    InvMassScale0 = 1.0f;
    InvMassScale1 = 1.0f;
    InvInertiaScale0 = 1.0f;
    InvInertiaScale1 = 1.0f;

    bBroken = false;
    bCollisionEnabled = true;
    bVisualization = true;
    bDriveLimitsAreForces = false;
    bImprovedSlerp = false;
    bDisablePreprocessing = false;
    bEnableExtendedLimits = false;
    bGpuCompatible = false;
    bAlwaysUpdate = false;
    bDisableConstraint = false;
}

void UPhysXJointBase::BeginPlay()
{
    Super::BeginPlay();
    //CreateJoint();
}

void UPhysXJointBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    //ReleaseJoint();
    Super::EndPlay(EndPlayReason);
}