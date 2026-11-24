#include "BasePhysXCollider.h"
#include "Components/PhysXMaterial.h"
#include "PhysXTypeConversions.h"
#include "GameFramework/Actor.h"

UBasePhysXCollider::UBasePhysXCollider()
{
    PrimaryComponentTick.bCanEverTick = false;
    LocalPosition = FVector::ZeroVector;
    LocalRotation = FQuat::Identity;
    Material = nullptr;
    ColliderShape = nullptr;
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

    /* cleanup?
    if (ColliderShape)
    {
        ColliderShape->release();
        ColliderShape = nullptr;
    }
    */
}

void UBasePhysXCollider::InitializeCollider(physx::PxPhysics* PxPhysics, physx::PxRigidActor* RigidActor, physx::PxMaterial* DefaultMaterial)
{
    if (!PxPhysics || !RigidActor)
        return;

    physx::PxGeometry* Geometry = CreateGeometry(PxPhysics);
    if (!Geometry)
        return;

    physx::PxMaterial* TransientMaterial = nullptr;
    physx::PxMaterial* ShapeMaterial = ResolveMaterial(PxPhysics, DefaultMaterial, TransientMaterial);
    if (!ShapeMaterial)
    {
        delete Geometry;
        return;
    }

    ColliderShape = PxPhysics->createShape(*Geometry, *ShapeMaterial, true);
    delete Geometry;

    if (ColliderShape)
    {
        ColliderShape->setLocalPose(physx::PxTransform(UEPhysX::ToPxVec3(LocalPosition), UEPhysX::ToPxQuat(LocalRotation)));
        ColliderShape->setContactOffset(ContactOffset);
        ColliderShape->setRestOffset(RestOffset);
        RigidActor->attachShape(*ColliderShape);
        ColliderShape->release();
    }

    if (TransientMaterial)
    {
        TransientMaterial->release();
    }
}

FTransform UBasePhysXCollider::MakePhysXTransform()
{
    const AActor* actor = GetOwner();
    return actor ? actor->GetActorTransform() : FTransform::Identity;
}

physx::PxMaterial* UBasePhysXCollider::ResolveMaterial(physx::PxPhysics* PxPhysics, physx::PxMaterial* DefaultMaterial, physx::PxMaterial*& OutTransientMaterial) const
{
    OutTransientMaterial = nullptr;

    if (Material)
    {
        return Material->GetOrCreateMaterial(*PxPhysics);
    }

    if (const AActor* Owner = GetOwner())
    {
        if (UPhysXMaterial* OwnerMaterial = Owner->FindComponentByClass<UPhysXMaterial>())
        {
            return OwnerMaterial->GetOrCreateMaterial(*PxPhysics);
        }
    }

    if (DefaultMaterial)
    {
        return DefaultMaterial;
    }

    OutTransientMaterial = PxPhysics->createMaterial(0.5f, 0.5f, 0.1f);
    return OutTransientMaterial;
}
