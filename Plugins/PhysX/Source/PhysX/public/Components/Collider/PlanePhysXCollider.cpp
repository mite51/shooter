#include "PlanePhysXCollider.h"

// Temporarily undefine Unreal's check macro to avoid conflicts with PhysX
#pragma push_macro("check")
#undef check

#include "foundation/PxMathUtils.h"

// Restore Unreal's check macro
#pragma pop_macro("check")

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