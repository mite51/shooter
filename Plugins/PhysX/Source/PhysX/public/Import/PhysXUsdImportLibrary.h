#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PhysXUsdImportLibrary.generated.h"

USTRUCT(BlueprintType)
struct FPhysXUsdImportOptions
{
	GENERATED_BODY()

	FPhysXUsdImportOptions();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|USD")
	float MetersToUnrealUnits;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|USD")
	bool bCreateSceneActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|USD")
	bool bCreateVisualComponents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|USD")
	bool bCreateJointComponents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|USD")
	bool bAutoInitializeScene;
};

USTRUCT(BlueprintType)
struct FPhysXUsdImportResult
{
	GENERATED_BODY()

	FPhysXUsdImportResult();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PhysX|USD")
	bool bSuccess;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PhysX|USD")
	FString Message;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PhysX|USD")
	AActor* RootActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PhysX|USD")
	int32 NumBodies;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PhysX|USD")
	int32 NumColliders;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PhysX|USD")
	int32 NumVisuals;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PhysX|USD")
	int32 NumJoints;
};

UCLASS()
class PHYSX_API UPhysXUsdImportLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "PhysX|USD", meta = (WorldContext = "WorldContextObject"))
	static FPhysXUsdImportResult ImportUsdPhysicsScene(UObject* WorldContextObject, const FString& UsdFilePath, const FPhysXUsdImportOptions& Options);
};
