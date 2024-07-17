#pragma once

#include "CoreMinimal.h"
#include "BasePhysXCollider.h"
#include "SpherePhysXCollider.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PHYSX_API USpherePhysXCollider : public UBasePhysXCollider
{
    GENERATED_BODY()

public:
    USpherePhysXCollider();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collider")
    float Radius;

protected:
    virtual physx::PxGeometry* CreateGeometry(physx::PxPhysics* PxPhysics) override;
};
