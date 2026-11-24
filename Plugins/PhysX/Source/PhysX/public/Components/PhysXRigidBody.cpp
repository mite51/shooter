#include "PhysXRigidBody.h"
#include "PhysXTypeConversions.h"
#include "GameFramework/Actor.h"

namespace
{
physx::PxTransform MakePxTransformFromActor(const AActor* Actor)
{
	return Actor ? UEPhysX::ToPxTransform(Actor->GetActorTransform()) : physx::PxTransform(physx::PxIdentity);
}
}

UPhysXRigidBody::UPhysXRigidBody()
{
    PrimaryComponentTick.bCanEverTick = true;
    RigidBody = nullptr;
    StaticBody = nullptr;

    // Set default values
    BodyType = EPhysXRigidBodyType::Dynamic;
    Mass = 1.0f;
    Density = 1.0f;
    bComputeMassFromShapes = false;
    MassSpaceInertiaTensor = FVector(1.0f, 1.0f, 1.0f);
    LinearDamping = 0.0f;
    AngularDamping = 0.0f;
    MaxLinearVelocity = 1000.0f;
    MaxAngularVelocity = 100.0f;
    LinearVelocity = FVector::ZeroVector;
    AngularVelocity = FVector::ZeroVector;
    bDisableGravity = false;
    bStartAwake = true;
    KINEMATIC = false;
    USE_KINEMATIC_TARGET_FOR_SCENE_QUERIES = false;
    ENABLE_CCD = false;
    ENABLE_CCD_FRICTION = false;
    ENABLE_POSE_INTEGRATION_PREVIEW = false;
    ENABLE_SPECULATIVE_CCD = false;
    ENABLE_CCD_MAX_CONTACT_IMPULSE = false;
    RETAIN_ACCELERATIONS = false;
}

void UPhysXRigidBody::CreateRigidBody(physx::PxPhysics* PxPhysics)
{
    if (PxPhysics && !RigidBody && !StaticBody)
    {
        physx::PxTransform T = MakePxTransformFromActor(GetOwner());
        if (BodyType == EPhysXRigidBodyType::Static)
        {
            StaticBody = PxPhysics->createRigidStatic(T);
            if (StaticBody)
            {
                StaticBody->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, bDisableGravity);
            }
        }
        else
        {
            RigidBody = PxPhysics->createRigidDynamic(T);
            SyncToPhysX();
        }
    }
}

void UPhysXRigidBody::FinalizeRigidBody()
{
    if (RigidBody && bComputeMassFromShapes && Density > 0.0f)
    {
        physx::PxRigidBodyExt::updateMassAndInertia(*RigidBody, Density);
        SyncFromPhysX();
    }
}

void UPhysXRigidBody::DestroyRigidBody()
{
    if (RigidBody)
    {
        RigidBody->release();
        RigidBody = nullptr;
    }

    if (StaticBody)
    {
        StaticBody->release();
        StaticBody = nullptr;
    }
}

physx::PxRigidActor* UPhysXRigidBody::GetPhysXRigidActor() const
{
    return RigidBody ? static_cast<physx::PxRigidActor*>(RigidBody) : static_cast<physx::PxRigidActor*>(StaticBody);
}

void UPhysXRigidBody::SyncTransformFromPhysX()
{
    // Update owner's transform based on RigidBody's transform
    if (physx::PxRigidActor* RigidActor = GetPhysXRigidActor())
    {
        const FTransform NewTransform = UEPhysX::FromPxTransform(RigidActor->getGlobalPose());
        GetOwner()->SetActorTransform(NewTransform);
    }
}

void UPhysXRigidBody::SyncTransformToPhysX()
{
    if (physx::PxRigidActor* RigidActor = GetPhysXRigidActor())
    {
        // Set the global pose of the rigid body
        physx::PxTransform T = MakePxTransformFromActor(GetOwner());
        RigidActor->setGlobalPose(T);
    }
}

void UPhysXRigidBody::SyncToPhysX()
{
    if (RigidBody)
    {
        RigidBody->setMass(Mass);
        RigidBody->setMassSpaceInertiaTensor(physx::PxVec3(MassSpaceInertiaTensor.X, MassSpaceInertiaTensor.Y, MassSpaceInertiaTensor.Z));
        RigidBody->setLinearDamping(LinearDamping);
        RigidBody->setAngularDamping(AngularDamping);
        RigidBody->setMaxLinearVelocity(MaxLinearVelocity);
        RigidBody->setMaxAngularVelocity(MaxAngularVelocity);
        RigidBody->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, bDisableGravity);

        //PxRigidBodyFlags
        int iflags = 0;
        const bool bKinematicBody = KINEMATIC || BodyType == EPhysXRigidBodyType::Kinematic;
        iflags |= bKinematicBody * (int)physx::PxRigidBodyFlag::Enum::eKINEMATIC;
        iflags |= USE_KINEMATIC_TARGET_FOR_SCENE_QUERIES * (int)physx::PxRigidBodyFlag::Enum::eUSE_KINEMATIC_TARGET_FOR_SCENE_QUERIES;
        iflags |= ENABLE_CCD * (int)physx::PxRigidBodyFlag::Enum::eENABLE_CCD;
        iflags |= ENABLE_CCD_FRICTION * (int)physx::PxRigidBodyFlag::Enum::eENABLE_CCD_FRICTION;
        iflags |= ENABLE_POSE_INTEGRATION_PREVIEW * (int)physx::PxRigidBodyFlag::Enum::eENABLE_POSE_INTEGRATION_PREVIEW;
        iflags |= ENABLE_SPECULATIVE_CCD * (int)physx::PxRigidBodyFlag::Enum::eENABLE_SPECULATIVE_CCD;
        iflags |= ENABLE_CCD_MAX_CONTACT_IMPULSE * (int)physx::PxRigidBodyFlag::Enum::eENABLE_CCD_MAX_CONTACT_IMPULSE;
        iflags |= RETAIN_ACCELERATIONS * (int)physx::PxRigidBodyFlag::Enum::eRETAIN_ACCELERATIONS;
        physx::PxRigidBodyFlag::Enum flags = static_cast<physx::PxRigidBodyFlag::Enum>(iflags);
        RigidBody->setRigidBodyFlags(flags);

        RigidBody->setLinearVelocity(physx::PxVec3(LinearVelocity.X, LinearVelocity.Y, LinearVelocity.Z), bStartAwake);
        RigidBody->setAngularVelocity(physx::PxVec3(AngularVelocity.X, AngularVelocity.Y, AngularVelocity.Z), bStartAwake);

        //SyncTransformToPhysX();
    }
}

void UPhysXRigidBody::SyncFromPhysX()
{
    if (RigidBody)
    {
        Mass = RigidBody->getMass();
        physx::PxVec3 InertiaTensor = RigidBody->getMassSpaceInertiaTensor();
        MassSpaceInertiaTensor = FVector(InertiaTensor.x, InertiaTensor.y, InertiaTensor.z);
        LinearDamping = RigidBody->getLinearDamping();
        AngularDamping = RigidBody->getAngularDamping();
        MaxLinearVelocity = RigidBody->getMaxLinearVelocity();
        MaxAngularVelocity = RigidBody->getMaxAngularVelocity();
        physx::PxVec3 LinVel = RigidBody->getLinearVelocity();
        LinearVelocity = FVector(LinVel.x, LinVel.y, LinVel.z);
        physx::PxVec3 AngVel = RigidBody->getAngularVelocity();
        AngularVelocity = FVector(AngVel.x, AngVel.y, AngVel.z);

        //PxRigidBodyFlags
        physx::PxRigidBodyFlags RigidBodyFlags = RigidBody->getRigidBodyFlags();
        KINEMATIC = RigidBodyFlags.isSet(static_cast<physx::PxRigidBodyFlag::Enum>(physx::PxRigidBodyFlag::Enum::eKINEMATIC));
        BodyType = KINEMATIC ? EPhysXRigidBodyType::Kinematic : EPhysXRigidBodyType::Dynamic;
        USE_KINEMATIC_TARGET_FOR_SCENE_QUERIES = RigidBodyFlags.isSet(static_cast<physx::PxRigidBodyFlag::Enum>(physx::PxRigidBodyFlag::Enum::eUSE_KINEMATIC_TARGET_FOR_SCENE_QUERIES));
        ENABLE_CCD = RigidBodyFlags.isSet(static_cast<physx::PxRigidBodyFlag::Enum>(physx::PxRigidBodyFlag::Enum::eENABLE_CCD));
        ENABLE_CCD_FRICTION = RigidBodyFlags.isSet(static_cast<physx::PxRigidBodyFlag::Enum>(physx::PxRigidBodyFlag::Enum::eENABLE_CCD_FRICTION));
        ENABLE_POSE_INTEGRATION_PREVIEW = RigidBodyFlags.isSet(static_cast<physx::PxRigidBodyFlag::Enum>(physx::PxRigidBodyFlag::Enum::eENABLE_POSE_INTEGRATION_PREVIEW));
        ENABLE_SPECULATIVE_CCD = RigidBodyFlags.isSet(static_cast<physx::PxRigidBodyFlag::Enum>(physx::PxRigidBodyFlag::Enum::eENABLE_SPECULATIVE_CCD));
        ENABLE_CCD_MAX_CONTACT_IMPULSE = RigidBodyFlags.isSet(static_cast<physx::PxRigidBodyFlag::Enum>(physx::PxRigidBodyFlag::Enum::eENABLE_CCD_MAX_CONTACT_IMPULSE));
        RETAIN_ACCELERATIONS = RigidBodyFlags.isSet(static_cast<physx::PxRigidBodyFlag::Enum>(physx::PxRigidBodyFlag::Enum::eRETAIN_ACCELERATIONS));

        //SyncTransformFromPhysX();
    }
}

void UPhysXRigidBody::AddForce(const FVector& Force, bool bAutoWake)
{
    if (RigidBody)
    {
        RigidBody->addForce(physx::PxVec3(Force.X, Force.Y, Force.Z), physx::PxForceMode::eFORCE, bAutoWake);
    }
}

void UPhysXRigidBody::AddTorque(const FVector& Torque, bool bAutoWake)
{
    if (RigidBody)
    {
        RigidBody->addTorque(physx::PxVec3(Torque.X, Torque.Y, Torque.Z), physx::PxForceMode::eFORCE, bAutoWake);
    }
}

