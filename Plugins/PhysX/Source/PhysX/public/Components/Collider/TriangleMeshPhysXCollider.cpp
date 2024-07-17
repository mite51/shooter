#include "TriangleMeshPhysXCollider.h"
#include "PhysXPublic.h"
#include "PxPhysicsAPI.h"
#include "cooking/PxCooking.h"

UTriangleMeshPhysXCollider::UTriangleMeshPhysXCollider()
{
    Scale = FVector(1.0f, 1.0f, 1.0f);
}

physx::PxGeometry* UTriangleMeshPhysXCollider::CreateGeometry(physx::PxPhysics* PxPhysics)
{
    if (!TriangleMesh)
        return nullptr;

    physx::PxTriangleMesh* PxTriMesh = CreatePxTriangleMesh(PxPhysics);
    if (!PxTriMesh)
        return nullptr;

    physx::PxMeshScale PxScale(physx::PxVec3(Scale.X, Scale.Y, Scale.Z));
    return new physx::PxTriangleMeshGeometry(PxTriMesh, PxScale);
}

physx::PxTriangleMesh* UTriangleMeshPhysXCollider::CreatePxTriangleMesh(physx::PxPhysics* PxPhysics)
{
    if (!TriangleMesh || !TriangleMesh->GetRenderData() || TriangleMesh->GetRenderData()->LODResources.Num() == 0)
        return nullptr;

    const FStaticMeshLODResources& LODResource = TriangleMesh->GetRenderData()->LODResources[0];
    const int32 NumVerts = LODResource.VertexBuffers.PositionVertexBuffer.GetNumVertices();
    const int32 NumTriangles = LODResource.IndexBuffer.GetNumIndices() / 3;

    TArray<physx::PxVec3> Vertices;
    Vertices.Reserve(NumVerts);

    for (int32 VertIdx = 0; VertIdx < NumVerts; VertIdx++)
    {
        const FVector3f& Position = LODResource.VertexBuffers.PositionVertexBuffer.VertexPosition(VertIdx);
        Vertices.Add(physx::PxVec3(Position.X, Position.Y, Position.Z));
    }

    TArray<uint32> Indices;
    Indices.Reserve(NumTriangles * 3);

    for (int32 TriIdx = 0; TriIdx < NumTriangles; TriIdx++)
    {
        Indices.Add(LODResource.IndexBuffer.GetIndex(TriIdx * 3 + 0));
        Indices.Add(LODResource.IndexBuffer.GetIndex(TriIdx * 3 + 1));
        Indices.Add(LODResource.IndexBuffer.GetIndex(TriIdx * 3 + 2));
    }

    physx::PxTriangleMeshDesc TriMeshDesc;
    TriMeshDesc.points.count = Vertices.Num();
    TriMeshDesc.points.stride = sizeof(physx::PxVec3);
    TriMeshDesc.points.data = Vertices.GetData();
    TriMeshDesc.triangles.count = NumTriangles;
    TriMeshDesc.triangles.stride = 3 * sizeof(uint32);
    TriMeshDesc.triangles.data = Indices.GetData();

    physx::PxTolerancesScale scale;
    physx::PxCookingParams params(scale);
    params.midphaseDesc.mBVH33Desc.meshCookingHint = physx::PxMeshCookingHint::eSIM_PERFORMANCE;
    params.midphaseDesc.mBVH33Desc.meshSizePerformanceTradeOff = 0.55f;

    physx::PxTriangleMesh* TriMesh = PxCreateTriangleMesh(params, TriMeshDesc, PxPhysics->getPhysicsInsertionCallback());

    return TriMesh;
}