#include "ConvexMeshPhysXCollider.h"
#include "PhysXPublic.h"
#include "cooking/PxCooking.h"
#include "Engine/StaticMesh.h"

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
    if (!ConvexMesh || !ConvexMesh->GetRenderData() || ConvexMesh->GetRenderData()->LODResources.Num() == 0)
        return nullptr;

    const FStaticMeshLODResources& LODResource = ConvexMesh->GetRenderData()->LODResources[0];
    const int32 NumVerts = LODResource.VertexBuffers.PositionVertexBuffer.GetNumVertices();

    TArray<physx::PxVec3> Vertices;
    Vertices.Reserve(NumVerts);

    for (int32 VertIdx = 0; VertIdx < NumVerts; VertIdx++)
    {
        const FVector3f& Position = LODResource.VertexBuffers.PositionVertexBuffer.VertexPosition(VertIdx);
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