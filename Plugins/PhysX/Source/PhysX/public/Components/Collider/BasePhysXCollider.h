#pragma once

#include "Components/ActorComponent.h"

#include "BasePhysXCollider.generated.h"

class UPhysXMaterial;
namespace physx
{
	class PxGeometry;
	class PxMaterial;
	class PxPhysics;
	class PxRigidActor;
	class PxShape;
}

UCLASS(Abstract, ClassGroup = (PhysX), meta = (BlueprintSpawnableComponent))
class PHYSX_API UBasePhysXCollider : public UActorComponent
{
    GENERATED_BODY()

    friend class UPhysXScene;

public:
    UBasePhysXCollider();

    virtual void InitializeCollider(physx::PxPhysics* PxPhysics, physx::PxRigidActor* RigidActor, physx::PxMaterial* DefaultMaterial = nullptr);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|Collider")
    FVector LocalPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|Collider")
    FQuat LocalRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|Collider")
    UPhysXMaterial* Material;

    virtual FTransform MakePhysXTransform();

    physx::PxShape* GetColliderShape() const { return ColliderShape; }

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    physx::PxShape* ColliderShape;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|Collider")
    float ContactOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|Collider")
    float RestOffset;

    virtual physx::PxGeometry* CreateGeometry(physx::PxPhysics* PxPhysics) PURE_VIRTUAL(UBasePhysXCollider::CreateGeometry, return nullptr;);

    physx::PxMaterial* ResolveMaterial(physx::PxPhysics* PxPhysics, physx::PxMaterial* DefaultMaterial, physx::PxMaterial*& OutTransientMaterial) const;
};
