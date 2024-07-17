#include "PhysXRigidBody.h"

UPhysXRigidBody::UPhysXRigidBody()
{
    PrimaryComponentTick.bCanEverTick = true;
    RigidBody = nullptr;

    // Set default values
    Mass = 1.0f;
    MassSpaceInertiaTensor = FVector(1.0f, 1.0f, 1.0f);
    LinearDamping = 0.0f;
    AngularDamping = 0.0f;
    MaxLinearVelocity = 1000.0f;
    MaxAngularVelocity = 100.0f;
    LinearVelocity = FVector::ZeroVector;
    AngularVelocity = FVector::ZeroVector;
}

void UPhysXRigidBody::CreateRigidBody(physx::PxPhysics* PxPhysics)
{
    if (PxPhysics && !RigidBody)
    {
        physx::PxTransform T = MakePxTransform();
        RigidBody = PxPhysics->createRigidDynamic(T);
        SyncToPhysX();
    }
}

void UPhysXRigidBody::DestroyRigidBody()
{
    if (RigidBody)
    {
        RigidBody->release();
        RigidBody = nullptr;
    }
}

void UPhysXRigidBody::SyncTransformFromPhysX()
{
    // Update owner's transform based on RigidBody's transform
    if (RigidBody)
    {
        physx::PxTransform PxTransform = RigidBody->getGlobalPose();
        FTransform NewTransform(FQuat(PxTransform.q.x, PxTransform.q.y, PxTransform.q.z, PxTransform.q.w),
            FVector(PxTransform.p.x, PxTransform.p.y, PxTransform.p.z));
        NewTransform.SetScale3D(FVector::One());//??needed?
        GetOwner()->SetActorTransform(NewTransform);
    }
}

physx::PxTransform UPhysXRigidBody::MakePxTransform(FTransform transform)
{
    // Store the location and rotation
    const FVector& location = transform.GetLocation();
    const FQuat& rotation = transform.GetRotation();

    // Convert Unreal's FQuat to PhysX's PxQuat
    physx::PxQuat px_Rotation(
        rotation.X,
        rotation.Y,
        rotation.Z,
        rotation.W
    );

    // Convert Unreal's FVector to PhysX's PxVec3
    physx::PxVec3 px_Position(
        location.X,
        location.Y,
        location.Z
    );

    // Create the PhysX transform
    return physx::PxTransform(px_Position, px_Rotation);
}

physx::PxTransform UPhysXRigidBody::MakePxTransform()
{
    const AActor* actor = GetOwner();
    FTransform transform = actor->GetActorTransform();
    return MakePxTransform(transform);
}

void UPhysXRigidBody::SyncTransformToPhysX()
{
    if (RigidBody)
    {
        // Set the global pose of the rigid body
        physx::PxTransform T = MakePxTransform();
        RigidBody->setGlobalPose(T);
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

        //PxRigidBodyFlags
        int iflags = 0;
        iflags |= KINEMATIC * (int)physx::PxRigidBodyFlag::Enum::eKINEMATIC;
        iflags |= ENABLE_CCD * (int)physx::PxRigidBodyFlag::Enum::eENABLE_CCD;
        iflags |= ENABLE_CCD_FRICTION * (int)physx::PxRigidBodyFlag::Enum::eENABLE_CCD_FRICTION;
        iflags |= ENABLE_POSE_INTEGRATION_PREVIEW * (int)physx::PxRigidBodyFlag::Enum::eENABLE_POSE_INTEGRATION_PREVIEW;
        iflags |= ENABLE_SPECULATIVE_CCD * (int)physx::PxRigidBodyFlag::Enum::eENABLE_SPECULATIVE_CCD;
        iflags |= ENABLE_CCD_MAX_CONTACT_IMPULSE * (int)physx::PxRigidBodyFlag::Enum::eENABLE_CCD_MAX_CONTACT_IMPULSE;
        iflags |= RETAIN_ACCELERATIONS * (int)physx::PxRigidBodyFlag::Enum::eRETAIN_ACCELERATIONS;
        physx::PxRigidBodyFlag::Enum flags = static_cast<physx::PxRigidBodyFlag::Enum>(iflags);
        RigidBody->setRigidBodyFlags(flags);

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

