#include "PlanePhysXCollider.h"
#include "foundation/PxMathUtils.h"

UPlanePhysXCollider::UPlanePhysXCollider()
{
}

physx::PxTransform UPlanePhysXCollider::MakePhysXTransform()
{
    FTransform T = GetOwner()->GetActorTransform();
    FQuat q = T.GetRotation();
    FVector up = q.GetUpVector();
    //return physx::PxTransformFromPlaneEquation(physx::PxPlane(q.X, q.Y, q.Z, q.W));
    return physx::PxTransformFromPlaneEquation(physx::PxPlane(up.X, up.Y, up.Z, 0.0f));
}


physx::PxGeometry* UPlanePhysXCollider::CreateGeometry(physx::PxPhysics* PxPhysics)
{
    return new physx::PxPlaneGeometry();
}