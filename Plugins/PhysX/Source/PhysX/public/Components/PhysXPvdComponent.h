#pragma once

#include "Components/ActorComponent.h"
#include "PhysXPvdComponent.generated.h"

UCLASS(ClassGroup = (PhysX), meta = (BlueprintSpawnableComponent))
class PHYSX_API UPhysXPvdComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPhysXPvdComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|PVD")
	bool bConnectToPvd;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|PVD")
	FString Host;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|PVD", meta = (ClampMin = "1", ClampMax = "65535"))
	int32 Port;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|PVD", meta = (ClampMin = "0"))
	int32 TimeoutMilliseconds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|PVD")
	bool bTransmitConstraints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|PVD")
	bool bTransmitContacts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|PVD")
	bool bTransmitSceneQueries;

	uint8 MakeInstrumentationFlagsMask() const;
};
