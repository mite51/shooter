#include "PlanePhysXCollider.h"

#include "NvidiaPhysXIncludes.h"
#include "GameFramework/Actor.h"

UPlanePhysXCollider::UPlanePhysXCollider()
{
}

FTransform UPlanePhysXCollider::MakePhysXTransform()
{
    const AActor* Owner = GetOwner();
    return Owner ? Owner->GetActorTransform() : FTransform::Identity;
}


physx::PxGeometry* UPlanePhysXCollider::CreateGeometry(physx::PxPhysics* PxPhysics)
{
    return new physx::PxPlaneGeometry();
}
