#pragma once

#include <PxPhysicsAPI.h>
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PhysXPublic.h"
#include "PxRigidBody.h"
#include "PhysXRigidBodyComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PHYSX_API UPhysXRigidBodyComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPhysXRigidBodyComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|RigidBody")
    float Mass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|RigidBody")
    FVector MassSpaceInertiaTensor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|RigidBody")
    float LinearDamping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|RigidBody")
    float AngularDamping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|RigidBody")
    float MaxLinearVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|RigidBody")
    float MaxAngularVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|RigidBody")
    FVector LinearVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|RigidBody")
    FVector AngularVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|RigidBody", meta = (DisplayName = "Kinematic"))
    bool KINEMATIC;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|RigidBody", meta = (DisplayName = "Use Kinematic Target for Scene Queries"))
    bool USE_KINEMATIC_TARGET_FOR_SCENE_QUERIES;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|RigidBody", meta = (DisplayName = "Enable CCD"))
    bool ENABLE_CCD;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|RigidBody", meta = (DisplayName = "Enable CCD Friction"))
    bool ENABLE_CCD_FRICTION;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|RigidBody", meta = (DisplayName = "Enable Pose Integration Preview"))
    bool ENABLE_POSE_INTEGRATION_PREVIEW;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|RigidBody", meta = (DisplayName = "Enable Speculative CCD"))
    bool ENABLE_SPECULATIVE_CCD;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|RigidBody", meta = (DisplayName = "Enable CCD Max Contact Impulse"))
    bool ENABLE_CCD_MAX_CONTACT_IMPULSE;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|RigidBody", meta = (DisplayName = "Retain Accelerations"))
    bool RETAIN_ACCELERATIONS;


    UFUNCTION(BlueprintCallable, Category = "PhysX|RigidBody")
    void CreateRigidBody();

    UFUNCTION(BlueprintCallable, Category = "PhysX|RigidBody")
    void DestroyRigidBody();

    UFUNCTION(BlueprintCallable, Category = "PhysX|RigidBody")
    void SyncToPhysX();

    UFUNCTION(BlueprintCallable, Category = "PhysX|RigidBody")
    void SyncFromPhysX();

    UFUNCTION(BlueprintCallable, Category = "PhysX|RigidBody")
    void AddForce(const FVector& Force, bool bAutoWake = true);

    UFUNCTION(BlueprintCallable, Category = "PhysX|RigidBody")
    void AddTorque(const FVector& Torque, bool bAutoWake = true);

private:
    physx::PxRigidBody* RigidBody;
    physx::PxPhysics* PhysXSDK;

    void InitializePhysXSDK();
    void CleanupPhysXSDK();
};
