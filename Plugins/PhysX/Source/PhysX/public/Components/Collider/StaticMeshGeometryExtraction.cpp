#include "StaticMeshGeometryExtraction.h"

#include "Engine/StaticMesh.h"
#include "StaticMeshResources.h"

namespace UEPhysX
{
    bool ExtractStaticMeshLOD0(const UStaticMesh* StaticMesh, TArray<FVector3f>& OutVertices, TArray<uint32>& OutIndices)
    {
        OutVertices.Reset();
        OutIndices.Reset();

        if (!StaticMesh || !StaticMesh->GetRenderData() || StaticMesh->GetRenderData()->LODResources.Num() == 0)
        {
            return false;
        }

        const FStaticMeshLODResources& LODResource = StaticMesh->GetRenderData()->LODResources[0];
        const int32 NumVerts = LODResource.VertexBuffers.PositionVertexBuffer.GetNumVertices();
        if (NumVerts <= 0)
        {
            return false;
        }

        OutVertices.Reserve(NumVerts);
        for (int32 VertIdx = 0; VertIdx < NumVerts; ++VertIdx)
        {
            OutVertices.Add(LODResource.VertexBuffers.PositionVertexBuffer.VertexPosition(VertIdx));
        }

        const int32 NumIndices = LODResource.IndexBuffer.GetNumIndices();
        OutIndices.Reserve(NumIndices);
        for (int32 Index = 0; Index < NumIndices; ++Index)
        {
            OutIndices.Add(LODResource.IndexBuffer.GetIndex(Index));
        }

        return OutVertices.Num() > 0;
    }
}
