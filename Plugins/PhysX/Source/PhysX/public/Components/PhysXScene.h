#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "PhysXScene.generated.h"

class UPhysXPvdComponent;
namespace physx
{
	class PxDefaultCpuDispatcher;
	class PxFoundation;
	class PxMaterial;
	class PxPhysics;
	class PxPvd;
	class PxPvdTransport;
	class PxRigidActor;
	class PxScene;
	class PxSceneDesc;
}

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

UCLASS(ClassGroup = (PhysX), meta = (BlueprintSpawnableComponent))
class PHYSX_API UPhysXScene : public UActorComponent
{
    GENERATED_BODY()

public:
    UPhysXScene();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    void InitializePhysXSimulation();
    void ShutdownPhysXSimulation();

    UFUNCTION(BlueprintCallable, Category = "PhysX")
    void StepPhysXSimulation(float DeltaTime);

    physx::PxFoundation* GetPxFoundation() const { return mFoundation; }
    physx::PxPhysics* GetPxPhysics() const { return mPhysics; }
    physx::PxScene* GetPxScene() const { return mScene; }
    physx::PxMaterial* GetDefaultMaterial() const { return mDefaultMaterial; }

    UFUNCTION(BlueprintCallable, Category = "PhysX")
    bool IsSimulationInitialized() const { return mScene != nullptr; }

    static UPhysXScene* FindPhysXScene(const UObject* WorldContextObject);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|PVD")
    bool bEnablePvd;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|PVD", meta = (DisplayName = "PVD Host"))
    FString PVD_HOST_IP = "127.0.0.1";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|PVD", meta = (ClampMin = "1", ClampMax = "65535"))
    int32 PvdPort;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|PVD", meta = (ClampMin = "0"))
    int32 PvdTimeoutMilliseconds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|Scene")
    bool bAutoStepSimulation = true;

    // PxSceneFlag options
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|Scene Flags")
    bool bEnableActiveActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|Scene Flags")
    bool bEnableCCD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|Scene Flags")
    bool bDisableCCDResweep;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysX|Scene Flags")
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
    bool InitializePhysXRuntime(UPhysXPvdComponent* PvdSettings);
    bool StepPhysXRuntime(float DeltaTime);
    void ShutdownPhysXRuntime();
    physx::PxRigidActor* CreateStaticRigidActor(const FTransform& Transform) const;
    void AddRigidActorToScene(physx::PxRigidActor* RigidActor) const;
    void SyncPhysXTransforms();
    UPhysXPvdComponent* ResolvePvdSettings() const;
    void ConfigurePvdSceneClient(UPhysXPvdComponent* PvdSettings) const;
    void ReleasePvd();

private:
    TArray<AActor*> PhysXActors;

    physx::PxFoundation* mFoundation = nullptr;
    physx::PxPhysics* mPhysics = nullptr;
    physx::PxScene* mScene = nullptr;
    //physx::PxArray<PxConstraint*>* gConstraints;
    physx::PxPvd* mPvd = nullptr;
    physx::PxPvdTransport* mPvdTransport = nullptr;
    physx::PxDefaultCpuDispatcher* mDispatcher = nullptr;
    physx::PxMaterial* mDefaultMaterial = nullptr;

};

