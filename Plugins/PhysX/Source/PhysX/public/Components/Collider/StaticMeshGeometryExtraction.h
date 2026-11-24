#pragma once

#include "CoreMinimal.h"

class UStaticMesh;

namespace UEPhysX
{
    bool ExtractStaticMeshLOD0(const UStaticMesh* StaticMesh, TArray<FVector3f>& OutVertices, TArray<uint32>& OutIndices);
}
