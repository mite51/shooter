#pragma once

#include "CoreMinimal.h"
#include "BasePhysXCollider.h"
#include "TriangleMeshPhysXCollider.generated.h"

namespace physx
{
    class PxTriangleMesh;
}

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PHYSX_API UTriangleMeshPhysXCollider : public UBasePhysXCollider
{
    GENERATED_BODY()

public:
    UTriangleMeshPhysXCollider();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collider")
    class UStaticMesh* TriangleMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collider")
    FVector Scale;

protected:
    virtual physx::PxGeometry* CreateGeometry(physx::PxPhysics* PxPhysics) override;

private:
    physx::PxTriangleMesh* CreatePxTriangleMesh(physx::PxPhysics* PxPhysics);
};

