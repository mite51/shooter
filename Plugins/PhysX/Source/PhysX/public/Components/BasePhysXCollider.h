#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PxPhysicsAPI.h"
#include "BasePhysXCollider.generated.h"

UCLASS(Abstract, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PHYSX_API UBasePhysXCollider : public UActorComponent
{
    GENERATED_BODY()

    friend class UPhysXScene;

public:
    UBasePhysXCollider();

    virtual void InitializeCollider(physx::PxPhysics* PxPhysics, physx::PxRigidActor* RigidActor);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collider")
    FVector LocalPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collider")
    FQuat LocalRotation;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    physx::PxShape* ColliderShape;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collider")
    float ContactOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collider")
    float RestOffset;

    virtual physx::PxGeometry* CreateGeometry(physx::PxPhysics* PxPhysics) PURE_VIRTUAL(UBasePhysXCollider::CreateGeometry, return nullptr;);
};