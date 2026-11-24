#pragma once

#include "Components/ActorComponent.h"
#include "PhysXMaterial.generated.h"

namespace physx
{
	class PxMaterial;
	class PxPhysics;
}

UENUM(BlueprintType)
enum class EPhysXCombineMode : uint8
{
	Average UMETA(DisplayName = "Average"),
	Min UMETA(DisplayName = "Minimum"),
	Multiply UMETA(DisplayName = "Multiply"),
	Max UMETA(DisplayName = "Maximum")
};

UCLASS(ClassGroup = (PhysX), meta = (BlueprintSpawnableComponent))
class PHYSX_API UPhysXMaterial : public UActorComponent
{
	GENERATED_BODY()

public:
	UPhysXMaterial();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|Material", meta = (ClampMin = "0.0"))
	float StaticFriction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|Material", meta = (ClampMin = "0.0"))
	float DynamicFriction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|Material", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Restitution;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|Material", meta = (ClampMin = "0.0"))
	float Density;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|Material")
	EPhysXCombineMode FrictionCombineMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|Material")
	EPhysXCombineMode RestitutionCombineMode;

	physx::PxMaterial* GetOrCreateMaterial(physx::PxPhysics& Physics);

	UFUNCTION(BlueprintCallable, Category = "PhysX|Material")
	void ReleaseMaterial();

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void ApplyToMaterial();

	physx::PxMaterial* PxMaterial = nullptr;
};
