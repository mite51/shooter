#pragma once

#include "PxPhysicsAPI.h"
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PhysXPublic.h"
#include "UPhysXScene.generated.h"

// Enum definitions (place these outside the class definition)
UENUM(BlueprintType)
enum class EPxBroadPhaseType : uint8
{
    eSAP UMETA(DisplayName = "Sweep and Prune"),
    eMBP UMETA(DisplayName = "Multi Box Pruning"),
    ePABP UMETA(DisplayName = "Parallel AABB Pruning"),
    eGPU UMETA(DisplayName = "GPU Broad Phase")
};

UENUM(BlueprintType)
enum class EPxFrictionType : uint8
{
    ePATCH UMETA(DisplayName = "Patch"),
    eTWO_DIRECTIONAL UMETA(DisplayName = "Two Directional")
};

UENUM(BlueprintType)
enum class EPxSolverType : uint8
{
    ePGS UMETA(DisplayName = "Projected Gauss-Seidel"),
    eTGS UMETA(DisplayName = "Temporal Gauss-Seidel")
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PHYSX_API UPhysXScene : public UActorComponent
{
    GENERATED_BODY()

public:
    UPhysXScene();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "PhysX")
    void InitializePhysXSimulation();

    UFUNCTION(BlueprintCallable, Category = "PhysX")
    void StepPhysXSimulation(float DeltaTime);

    // PxSceneFlag options
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Flags")
    bool bEnableActiveActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Flags")
    bool bEnableCCD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Flags")
    bool bDisableCCDResweep;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Flags")
    bool bEnablePCM;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Flags")
    bool bDisableContactReportBufferResize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Flags")
    bool bDisableContactCache;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Flags")
    bool bRequireRWLock;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Flags")
    bool bEnableStabilization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Flags")
    bool bEnableAveragePoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Flags")
    bool bExcludeKinematicsFromActiveActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Flags")
    bool bEnableGPUDynamics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Flags")
    bool bEnableEnhancedDeterminism;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Flags")
    bool bEnableFrictionEveryIteration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Flags")
    bool bEnableDirectGPUAPI;

    // Other PxSceneDesc options
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Description")
    FVector Gravity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Description")
    EPxBroadPhaseType BroadPhaseType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Description")
    EPxFrictionType FrictionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Description")
    EPxSolverType SolverType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Description")
    float BounceThresholdVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Description")
    float FrictionOffsetThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Description")
    float FrictionCorrelationDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Description")
    int SolverBatchSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Description")
    int SolverArticulationBatchSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Description")
    int NbContactDataBlocks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Description")
    int MaxNbContactDataBlocks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Description")
    float MaxBiasCoefficient;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Description")
    int ContactReportStreamBufferSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Description")
    int CcdMaxPasses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Description")
    float CcdThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Description")
    float CcdMaxSeparation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Description")
    float WakeCounterResetValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Description")
    FVector SanityBoundsMin;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Description")
    FVector SanityBoundsMax;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Description")
    int GPUMaxNumPartitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Description")
    int GPUMaxNumStaticPartitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Description")
    int GPUComputeVersion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX Scene Description")
    int ContactPairSlabSize;

    //UFUNCTION(BlueprintCallable, Category = "PhysX")
    physx::PxSceneDesc CreateSceneDesc(physx::PxPhysics* Physics);

private:
    // Helper function to convert enum values to shift amounts
    static int FlagToShift(physx::PxSceneFlag::Enum flag);

    // Helper functions to convert UE4 enums to PhysX enums
    static physx::PxBroadPhaseType::Enum ToPxBroadPhaseType(EPxBroadPhaseType Type);
    static physx::PxFrictionType::Enum ToPxFrictionType(EPxFrictionType Type);
    static physx::PxSolverType::Enum ToPxSolverType(EPxSolverType Type);

    physx::PxFoundation* mFoundation;
    physx::PxPhysics* mPhysics;
    physx::PxScene* mScene;
};

