#pragma once

#include "CoreMinimal.h"
#include "BasePhysXCollider.h"
#include "PlanePhysXCollider.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PHYSX_API UPlanePhysXCollider : public UBasePhysXCollider
{
    GENERATED_BODY()

public:
    UPlanePhysXCollider();

protected:
    virtual physx::PxGeometry* CreateGeometry(physx::PxPhysics* PxPhysics) override;
};
