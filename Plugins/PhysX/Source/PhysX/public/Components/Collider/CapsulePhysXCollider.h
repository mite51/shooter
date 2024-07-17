#pragma once

#include "CoreMinimal.h"
#include "BasePhysXCollider.h"
#include "CapsulePhysXCollider.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PHYSX_API UCapsulePhysXCollider : public UBasePhysXCollider
{
    GENERATED_BODY()

public:
    UCapsulePhysXCollider();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collider")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collider")
    float HalfHeight;

protected:
    virtual physx::PxGeometry* CreateGeometry(physx::PxPhysics* PxPhysics) override;
};
