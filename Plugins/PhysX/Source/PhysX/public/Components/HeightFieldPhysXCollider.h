#pragma once

#include "CoreMinimal.h"
#include "BasePhysXCollider.h"
//#include "LandscapeComponent.h"
#include "HeightFieldPhysXCollider.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PHYSX_API UHeightFieldPhysXCollider : public UBasePhysXCollider
{
    GENERATED_BODY()

public:
    UHeightFieldPhysXCollider();

    UPROPERTY(EditAnywhere, Category = "Collider", meta = (UseComponentPicker, AllowedClasses = "LandscapeComponent"))
    class ULandscapeComponent* LandscapeComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collider")
    float HeightScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collider")
    float RowScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collider")
    float ColumnScale;

protected:
    virtual physx::PxGeometry* CreateGeometry(physx::PxPhysics* PxPhysics) override;

private:
    physx::PxHeightField* CreatePxHeightField(physx::PxPhysics* PxPhysics);
};
