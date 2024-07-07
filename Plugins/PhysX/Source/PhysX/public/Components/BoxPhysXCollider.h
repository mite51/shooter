#pragma once

#include "CoreMinimal.h"
#include "BasePhysXCollider.h"
#include "BoxPhysXCollider.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PHYSX_API UBoxPhysXCollider : public UBasePhysXCollider
{
    GENERATED_BODY()

public:
    UBoxPhysXCollider();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collider")
    FVector HalfExtents;

protected:
    virtual physx::PxGeometry* CreateGeometry(physx::PxPhysics* PxPhysics) override;
};
