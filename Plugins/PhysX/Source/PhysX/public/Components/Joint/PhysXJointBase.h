
#pragma once

#include "Engine/EngineTypes.h"
#include "PxPhysicsAPI.h"
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PhysXPublic.h"
#include "../PhysXRigidBody.h"
#include "PhysXJointBase.generated.h"

using namespace physx;

UENUM(BlueprintType)
enum class EPxD6Motion : uint8
{
    Locked   UMETA(DisplayName = "Locked"),
    Limited  UMETA(DisplayName = "Limited"),
    Free     UMETA(DisplayName = "Free")
};

UENUM(BlueprintType)
enum class EPxD6Axis : uint8
{
    X       UMETA(DisplayName = "X"),
    Y       UMETA(DisplayName = "Y"),
    Z       UMETA(DisplayName = "Z"),
    Twist   UMETA(DisplayName = "Twist"),
    Swing1  UMETA(DisplayName = "Swing1"),
    Swing2  UMETA(DisplayName = "Swing2")
};

UENUM(BlueprintType)
enum class EPxD6Drive : uint8
{
    X      UMETA(DisplayName = "X"),
    Y      UMETA(DisplayName = "Y"),
    Z      UMETA(DisplayName = "Z"),
    Swing  UMETA(DisplayName = "Swing"),
    Twist  UMETA(DisplayName = "Twist"),
    Slerp  UMETA(DisplayName = "Slerp")
};

USTRUCT(BlueprintType)
struct FPxD6JointDrive
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "D6Joint|Drive")
    float Stiffness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "D6Joint|Drive")
    float Damping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "D6Joint|Drive")
    float ForceLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "D6Joint|Drive")
    bool bAcceleration;

    FPxD6JointDrive()
        : Stiffness(0.0f)
        , Damping(0.0f)
        , ForceLimit(FLT_MAX)
        , bAcceleration(false)
    {}
};

UCLASS(Abstract, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PHYSX_API UPhysXJointBase : public UActorComponent
{
    GENERATED_BODY()

public:
    UPhysXJointBase();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint")//, meta = (UseComponentPicker, AllowedClasses = "UPhysXRigidBody"))
    AActor* Actor0;
    //???
    //UPhysXRigidBody* Actor0;
    //FComponentReference

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint")//, meta = (UseComponentPicker, AllowedClasses = "UPhysXRigidBody"))
    AActor* Actor1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint")
    FTransform LocalPose0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint")
    FTransform LocalPose1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint")
    float BreakForce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint")
    float BreakTorque;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint")
    float InvMassScale0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint")
    float InvMassScale1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint")
    float InvInertiaScale0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint")
    float InvInertiaScale1;

    // Constraint flags as individual booleans
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint|Flags")
    bool bBroken;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint|Flags")
    bool bCollisionEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint|Flags")
    bool bVisualization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint|Flags")
    bool bDriveLimitsAreForces;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint|Flags")
    bool bImprovedSlerp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint|Flags")
    bool bDisablePreprocessing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint|Flags")
    bool bEnableExtendedLimits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint|Flags")
    bool bGpuCompatible;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint|Flags")
    bool bAlwaysUpdate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint|Flags")
    bool bDisableConstraint;

    virtual void CreateJoint(physx::PxPhysics* PxPhysics) PURE_VIRTUAL(UPhysXJointBase::CreateJoint, );
    virtual void ReleaseJoint() PURE_VIRTUAL(UPhysXJointBase::ReleaseJoint, );

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
