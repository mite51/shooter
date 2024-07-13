#include "SpherePhysXCollider.h"

USpherePhysXCollider::USpherePhysXCollider()
{
    
}

physx::PxGeometry* USpherePhysXCollider::CreateGeometry(physx::PxPhysics* PxPhysics)
{
    return new physx::PxSphereGeometry(Radius);
}

