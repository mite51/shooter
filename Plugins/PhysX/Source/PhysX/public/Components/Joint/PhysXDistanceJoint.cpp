#include "PhysXDistanceJoint.h"
#include "PhysicsPublic.h"
#include "PhysXPublic.h"


#if WITH_EDITOR
#include "UnrealEd.h"
#include "PhysXDistanceJoint_Visualizer.h"
static bool gRegisteredVisualizer = false;
#endif


UPhysXDistanceJoint::UPhysXDistanceJoint()
    : DistanceJoint(nullptr)
{
#if WITH_EDITOR
    if (!gRegisteredVisualizer && GUnrealEd)
    {
        gRegisteredVisualizer = true;
        TSharedPtr<FComponentVisualizer> visualizer = MakeShareable(new PhysXDistanceJoint_Visualizer());
        GUnrealEd->RegisterComponentVisualizer(UPhysXDistanceJoint::StaticClass()->GetFName(), visualizer);
        visualizer->OnRegister();
    }
#endif

    MinDistance = 0.0f;
    MaxDistance = 0.0f;
    Tolerance = 0.25f;
    Stiffness = 0.0f;
    Damping = 0.0f;
    MaxDistanceEnabled = true;
    MinDistanceEnabled = false;
    SpringEnabled = false;

    bVisualizeJoint = true;
    VisualizationScale = 1.0f;
    JointColor = FColor::Yellow;
}

void UPhysXDistanceJoint::CreateJoint(PxPhysics* PxPhysics)
{
    if (!Actor0 || !Actor1 || !PxPhysics)
    {
        UE_LOG(LogPhysics, Warning, TEXT("Cannot create DistanceJoint: Invalid actors or PhysX SDK"));
        return;
    }

    UPhysXRigidBody* uRigidBody0 = Actor0->FindComponentByClass<UPhysXRigidBody>();
    UPhysXRigidBody* uRigidBody1 = Actor1->FindComponentByClass<UPhysXRigidBody>();

    PxRigidActor* PxActor0 = uRigidBody0->GetPhysXRigidActor();
    PxRigidActor* PxActor1 = uRigidBody1->GetPhysXRigidActor();

    if (!PxActor0 || !PxActor1)
    {
        UE_LOG(LogPhysics, Warning, TEXT("Cannot create DistanceJoint: Invalid PhysX actors"));
        return;
    }

    PxTransform PxLocalPose0 = PxTransform(PxIdentity);// uRigidBody0->MakePxTransform();
    PxTransform PxLocalPose1 = PxTransform(PxIdentity);// uRigidBody1->MakePxTransform();

    DistanceJoint = PxDistanceJointCreate(*PxPhysics, PxActor0, PxLocalPose0, PxActor1, PxLocalPose1);

    if (!DistanceJoint)
    {
        UE_LOG(LogPhysics, Warning, TEXT("Failed to create DistanceJoint"));
        return;
    }

    // Set up joint properties
    DistanceJoint->setMinDistance(MinDistance);
    DistanceJoint->setMaxDistance(MaxDistance);

    DistanceJoint->setTolerance(Tolerance);
    DistanceJoint->setStiffness(Stiffness);
    DistanceJoint->setDamping(Damping);

    // set flags
    int iflags = 0;
    iflags |= MaxDistanceEnabled * (int)physx::PxDistanceJointFlag::Enum::eMAX_DISTANCE_ENABLED;
    iflags |= MinDistanceEnabled * (int)physx::PxDistanceJointFlag::Enum::eMIN_DISTANCE_ENABLED;
    iflags |= SpringEnabled * (int)physx::PxDistanceJointFlag::Enum::eSPRING_ENABLED;
    physx::PxDistanceJointFlag::Enum flags = static_cast<physx::PxDistanceJointFlag::Enum>(iflags);
    DistanceJoint->setDistanceJointFlags(flags);

    DistanceJoint->setBreakForce(BreakForce, BreakTorque);

    PxConstraintFlags ConstraintFlags;
    ConstraintFlags |= bBroken ? PxConstraintFlag::eBROKEN : PxConstraintFlag::Enum(0);
    ConstraintFlags |= bCollisionEnabled ? PxConstraintFlag::eCOLLISION_ENABLED : PxConstraintFlag::Enum(0);
    ConstraintFlags |= bVisualization ? PxConstraintFlag::eVISUALIZATION : PxConstraintFlag::Enum(0);
    ConstraintFlags |= bGpuCompatible ? PxConstraintFlag::eGPU_COMPATIBLE : PxConstraintFlag::Enum(0);
    ConstraintFlags |= bAlwaysUpdate ? PxConstraintFlag::eALWAYS_UPDATE : PxConstraintFlag::Enum(0);
    ConstraintFlags |= bDisableConstraint ? PxConstraintFlag::eDISABLE_CONSTRAINT : PxConstraintFlag::Enum(0);

    DistanceJoint->setConstraintFlags(ConstraintFlags);

    DistanceJoint->setInvMassScale0(InvMassScale0);
    DistanceJoint->setInvMassScale1(InvMassScale1);
    DistanceJoint->setInvInertiaScale0(InvInertiaScale0);
    DistanceJoint->setInvInertiaScale1(InvInertiaScale1);

}

void UPhysXDistanceJoint::ReleaseJoint()
{
    if (DistanceJoint)
    {
        DistanceJoint->release();
        DistanceJoint = nullptr;
    }
}

float UPhysXDistanceJoint::GetDistance() const
{
    return DistanceJoint ? DistanceJoint->getDistance() : 0.0f;
}

void UPhysXDistanceJoint::SetMinDistance(float distance)
{
    MinDistance = distance;
    if (DistanceJoint)
    {
        DistanceJoint->setMinDistance(distance);
    }
}

void UPhysXDistanceJoint::SetMaxDistance(float distance)
{
    MaxDistance = distance;
    if (DistanceJoint)
    {
        DistanceJoint->setMaxDistance(distance);
    }
}

void UPhysXDistanceJoint::SetTolerance(float tolerance)
{
    Tolerance = tolerance;
    if (DistanceJoint)
    {
        DistanceJoint->setTolerance(tolerance);
    }
}

void UPhysXDistanceJoint::SetStiffness(float stiffness)
{
    Stiffness = stiffness;
    if (DistanceJoint)
    {
        DistanceJoint->setStiffness(stiffness);
    }
}

void UPhysXDistanceJoint::SetDamping(float damping)
{
    Damping = damping;
    if (DistanceJoint)
    {
        DistanceJoint->setDamping(damping);
    }
}

void UPhysXDistanceJoint::SetMaxDistanceEnabled(bool value)
{
    physx::PxDistanceJointFlags flags = DistanceJoint->getDistanceJointFlags();
    DistanceJoint->setDistanceJointFlags(flags | physx::PxDistanceJointFlag::Enum::eMAX_DISTANCE_ENABLED);
}

void UPhysXDistanceJoint::SetMinDistanceEnabled(bool value)
{
    physx::PxDistanceJointFlags flags = DistanceJoint->getDistanceJointFlags();
    DistanceJoint->setDistanceJointFlags(flags | physx::PxDistanceJointFlag::Enum::eMIN_DISTANCE_ENABLED);
}

void UPhysXDistanceJoint::SetSpringEnabled(bool value)
{
    physx::PxDistanceJointFlags flags = DistanceJoint->getDistanceJointFlags();
    DistanceJoint->setDistanceJointFlags(flags | physx::PxDistanceJointFlag::Enum::eSPRING_ENABLED);
}
