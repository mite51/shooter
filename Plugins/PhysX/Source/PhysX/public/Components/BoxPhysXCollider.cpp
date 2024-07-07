
#include "BoxPhysXCollider.h"

UBoxPhysXCollider::UBoxPhysXCollider()
{
    HalfExtents = FVector(50.0f, 50.0f, 50.0f);
}

physx::PxGeometry* UBoxPhysXCollider::CreateGeometry(physx::PxPhysics* PxPhysics)
{
    return new physx::PxBoxGeometry(HalfExtents.X, HalfExtents.Y, HalfExtents.Z);
}
