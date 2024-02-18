// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <PxPhysicsAPI.h>
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"


#include "PhysxActor.generated.h"

using namespace physx;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PHYSX_API UPhysxActor : public UActorComponent
{
	GENERATED_BODY()

private:
	PxRigidDynamic* dynamic;

public:	
	// Sets default values for this component's properties
	UPhysxActor();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
