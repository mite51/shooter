#pragma once

#include "CoreMinimal.h"
#include "NvidiaPhysXIncludes.h"

namespace UEPhysX
{
	inline physx::PxVec3 ToPxVec3(const FVector& Value)
	{
		return physx::PxVec3(Value.X, Value.Y, Value.Z);
	}

	inline FVector FromPxVec3(const physx::PxVec3& Value)
	{
		return FVector(Value.x, Value.y, Value.z);
	}

	inline physx::PxQuat ToPxQuat(const FQuat& Value)
	{
		return physx::PxQuat(Value.X, Value.Y, Value.Z, Value.W);
	}

	inline FQuat FromPxQuat(const physx::PxQuat& Value)
	{
		return FQuat(Value.x, Value.y, Value.z, Value.w);
	}

	inline physx::PxTransform ToPxTransform(const FTransform& Value)
	{
		return physx::PxTransform(ToPxVec3(Value.GetLocation()), ToPxQuat(Value.GetRotation()));
	}

	inline FTransform FromPxTransform(const physx::PxTransform& Value)
	{
		return FTransform(FromPxQuat(Value.q), FromPxVec3(Value.p), FVector::OneVector);
	}
}
