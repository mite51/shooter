#include "PlanePhysXCollider.h"

UPlanePhysXCollider::UPlanePhysXCollider()
{
}

physx::PxGeometry* UPlanePhysXCollider::CreateGeometry(physx::PxPhysics* PxPhysics)
{
    return new physx::PxPlaneGeometry();
}