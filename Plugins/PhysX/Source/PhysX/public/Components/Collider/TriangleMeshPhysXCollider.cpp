#include "TriangleMeshPhysXCollider.h"

#include "NvidiaPhysXIncludes.h"
#include "StaticMeshGeometryExtraction.h"
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
    TArray<FVector3f> MeshVertices;
    TArray<uint32> Indices;
    if (!UEPhysX::ExtractStaticMeshLOD0(TriangleMesh, MeshVertices, Indices) || Indices.Num() < 3)
    {
        return nullptr;
    }

    TArray<physx::PxVec3> Vertices;
    Vertices.Reserve(MeshVertices.Num());

    for (const FVector3f& Position : MeshVertices)
    {
        Vertices.Add(physx::PxVec3(Position.X, Position.Y, Position.Z));
    }

    const int32 NumTriangles = Indices.Num() / 3;

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
