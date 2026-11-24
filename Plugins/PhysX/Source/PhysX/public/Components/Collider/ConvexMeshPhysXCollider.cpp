#include "ConvexMeshPhysXCollider.h"

#include "NvidiaPhysXIncludes.h"
#include "StaticMeshGeometryExtraction.h"
#include "cooking/PxCooking.h"

UConvexMeshPhysXCollider::UConvexMeshPhysXCollider()
{
    Scale = FVector(1.0f, 1.0f, 1.0f);
}

physx::PxGeometry* UConvexMeshPhysXCollider::CreateGeometry(physx::PxPhysics* PxPhysics)
{
    if (!ConvexMesh)
        return nullptr;

    physx::PxConvexMesh* PxConvexMesh = CreatePxConvexMesh(PxPhysics);
    if (!PxConvexMesh)
        return nullptr;

    physx::PxMeshScale PxScale(physx::PxVec3(Scale.X, Scale.Y, Scale.Z));
    return new physx::PxConvexMeshGeometry(PxConvexMesh, PxScale);
}

physx::PxConvexMesh* UConvexMeshPhysXCollider::CreatePxConvexMesh(physx::PxPhysics* PxPhysics)
{
    TArray<FVector3f> MeshVertices;
    TArray<uint32> IgnoredIndices;
    if (!UEPhysX::ExtractStaticMeshLOD0(ConvexMesh, MeshVertices, IgnoredIndices))
    {
        return nullptr;
    }

    TArray<physx::PxVec3> Vertices;
    Vertices.Reserve(MeshVertices.Num());

    for (const FVector3f& Position : MeshVertices)
    {
        Vertices.Add(physx::PxVec3(Position.X, Position.Y, Position.Z));
    }

    physx::PxConvexMeshDesc ConvexDesc;
    ConvexDesc.points.count = Vertices.Num();
    ConvexDesc.points.stride = sizeof(physx::PxVec3);
    ConvexDesc.points.data = Vertices.GetData();
    ConvexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;

    physx::PxTolerancesScale pxScale;
    physx::PxCookingParams Params(pxScale);
    Params.convexMeshCookingType = physx::PxConvexMeshCookingType::eQUICKHULL;

    return PxCreateConvexMesh(Params, ConvexDesc, PxPhysics->getPhysicsInsertionCallback());
}
