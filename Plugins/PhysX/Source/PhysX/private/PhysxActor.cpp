// Fill out your copyright notice in the Description page of Project Settings.


#include "PhysxActor.h"

// Sets default values for this component's properties
UPhysxActor::UPhysxActor()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPhysxActor::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UPhysxActor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Some debug message!"));
}

