#include "BasePhysXCollider.h"

UBasePhysXCollider::UBasePhysXCollider()
{
    PrimaryComponentTick.bCanEverTick = false;
    ContactOffset = 0.02f; // Default PhysX value
    RestOffset = 0.0f;
}

void UBasePhysXCollider::BeginPlay()
{
    Super::BeginPlay();
}

void UBasePhysXCollider::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    if (ColliderShape)
    {
        ColliderShape->release();
        ColliderShape = nullptr;
    }
}

void UBasePhysXCollider::InitializeCollider(physx::PxPhysics* PxPhysics, physx::PxRigidActor* RigidActor)
{
    if (!PxPhysics || !RigidActor)
        return;

    physx::PxGeometry* Geometry = CreateGeometry(PxPhysics);
    if (!Geometry)
        return;

    ColliderShape = PxPhysics->createShape(*Geometry, *PxPhysics->createMaterial(0.5f, 0.5f, 0.1f), true);
    if (ColliderShape)
    {
        ColliderShape->setLocalPose(physx::PxTransform(physx::PxVec3(LocalPosition.X, LocalPosition.Y, LocalPosition.Z),
            physx::PxQuat(LocalRotation.X, LocalRotation.Y, LocalRotation.Z, LocalRotation.W)));
        ColliderShape->setContactOffset(ContactOffset);
        ColliderShape->setRestOffset(RestOffset);
        RigidActor->attachShape(*ColliderShape);
    }

    delete Geometry;
}


