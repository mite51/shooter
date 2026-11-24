#include "Components/PhysXMaterial.h"
#include "NvidiaPhysXIncludes.h"

namespace
{
physx::PxCombineMode::Enum ToPxCombineMode(EPhysXCombineMode Mode)
{
	switch (Mode)
	{
	case EPhysXCombineMode::Min:
		return physx::PxCombineMode::eMIN;
	case EPhysXCombineMode::Multiply:
		return physx::PxCombineMode::eMULTIPLY;
	case EPhysXCombineMode::Max:
		return physx::PxCombineMode::eMAX;
	case EPhysXCombineMode::Average:
	default:
		return physx::PxCombineMode::eAVERAGE;
	}
}
}

UPhysXMaterial::UPhysXMaterial()
{
	PrimaryComponentTick.bCanEverTick = false;

	StaticFriction = 0.5f;
	DynamicFriction = 0.5f;
	Restitution = 0.1f;
	Density = 1.0f;
	FrictionCombineMode = EPhysXCombineMode::Average;
	RestitutionCombineMode = EPhysXCombineMode::Average;
}

physx::PxMaterial* UPhysXMaterial::GetOrCreateMaterial(physx::PxPhysics& Physics)
{
	if (!PxMaterial)
	{
		PxMaterial = Physics.createMaterial(StaticFriction, DynamicFriction, Restitution);
	}

	ApplyToMaterial();
	return PxMaterial;
}

void UPhysXMaterial::ReleaseMaterial()
{
	if (PxMaterial)
	{
		PxMaterial->release();
		PxMaterial = nullptr;
	}
}

void UPhysXMaterial::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ReleaseMaterial();
	Super::EndPlay(EndPlayReason);
}

void UPhysXMaterial::ApplyToMaterial()
{
	if (!PxMaterial)
	{
		return;
	}

	PxMaterial->setStaticFriction(StaticFriction);
	PxMaterial->setDynamicFriction(DynamicFriction);
	PxMaterial->setRestitution(Restitution);
	PxMaterial->setFrictionCombineMode(ToPxCombineMode(FrictionCombineMode));
	PxMaterial->setRestitutionCombineMode(ToPxCombineMode(RestitutionCombineMode));
}
