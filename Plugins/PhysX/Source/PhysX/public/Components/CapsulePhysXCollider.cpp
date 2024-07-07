#include "CapsulePhysXCollider.h"

UCapsulePhysXCollider::UCapsulePhysXCollider()
{
    Radius = 25.0f;
    HalfHeight = 50.0f;
}

physx::PxGeometry* UCapsulePhysXCollider::CreateGeometry(physx::PxPhysics* PxPhysics)
{
    return new physx::PxCapsuleGeometry(Radius, HalfHeight);
}
