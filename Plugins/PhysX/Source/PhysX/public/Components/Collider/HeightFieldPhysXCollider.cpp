#include "HeightFieldPhysXCollider.h"

#include "NvidiaPhysXIncludes.h"

UHeightFieldPhysXCollider::UHeightFieldPhysXCollider()
{
    HeightScale = 1.0f;
    RowScale = 1.0f;
    ColumnScale = 1.0f;
}

physx::PxGeometry* UHeightFieldPhysXCollider::CreateGeometry(physx::PxPhysics* PxPhysics)
{
    return nullptr;
}

physx::PxHeightField* UHeightFieldPhysXCollider::CreatePxHeightField(physx::PxPhysics* PxPhysics)
{
    return nullptr;
}
