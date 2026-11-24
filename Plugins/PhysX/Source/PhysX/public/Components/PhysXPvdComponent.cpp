#include "Components/PhysXPvdComponent.h"
#include "NvidiaPhysXIncludes.h"

UPhysXPvdComponent::UPhysXPvdComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	bConnectToPvd = false;
	Host = TEXT("127.0.0.1");
	Port = 5425;
	TimeoutMilliseconds = 10;
	bTransmitConstraints = true;
	bTransmitContacts = true;
	bTransmitSceneQueries = true;
}

uint8 UPhysXPvdComponent::MakeInstrumentationFlagsMask() const
{
	uint8 Flags = 0;

	if (bTransmitConstraints)
	{
		Flags |= physx::PxPvdInstrumentationFlag::eDEBUG;
	}

	if (bTransmitContacts)
	{
		Flags |= physx::PxPvdInstrumentationFlag::ePROFILE;
	}

	if (bTransmitSceneQueries)
	{
		Flags |= physx::PxPvdInstrumentationFlag::eMEMORY;
	}

	return Flags;
}
