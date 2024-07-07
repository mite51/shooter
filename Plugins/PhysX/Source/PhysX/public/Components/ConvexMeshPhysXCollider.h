#pragma once

#include "CoreMinimal.h"
#include "BasePhysXCollider.h"
#include "ConvexMeshPhysXCollider.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PHYSX_API UConvexMeshPhysXCollider : public UBasePhysXCollider
{
    GENERATED_BODY()

public:
    UConvexMeshPhysXCollider();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collider")
    class UStaticMesh* ConvexMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collider")
    FVector Scale;

protected:
    virtual physx::PxGeometry* CreateGeometry(physx::PxPhysics* PxPhysics) override;

private:
    physx::PxConvexMesh* CreatePxConvexMesh(physx::PxPhysics* PxPhysics);
};

