#include "SpherePhysXCollider.h"

USpherePhysXCollider::USpherePhysXCollider()
{
    Radius = 50.0f;
}

physx::PxGeometry* USpherePhysXCollider::CreateGeometry(physx::PxPhysics* PxPhysics)
{
    return new physx::PxSphereGeometry(Radius);
}

