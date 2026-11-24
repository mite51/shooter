#include "PhysXScene.h"

#include "Collider/BasePhysXCollider.h"
#include "Components/PhysXPvdComponent.h"
#include "Joint/PhysXJointBase.h"
#include "PhysX.h"
#include "PhysXRigidBody.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

namespace
{
    constexpr float DefaultPhysXMaxFloat = 3.4028234663852885981170418348452e+38F;
    constexpr float DefaultPhysXMaxBoundsExtents = DefaultPhysXMaxFloat * 0.25f;

    void GetAllWorldActors(const UWorld* World, TArray<AActor*>& OutActors)
    {
        OutActors.Reset();

        if (!World)
        {
            return;
        }

        for (ULevel* Level : World->GetLevels())
        {
            if (!Level)
            {
                continue;
            }

            for (AActor* Actor : Level->Actors)
            {
                if (Actor)
                {
                    OutActors.Add(Actor);
                }
            }
        }
    }
}

UPhysXScene::UPhysXScene()
{
    PrimaryComponentTick.bCanEverTick = true;

    bEnablePvd = false;
    PVD_HOST_IP = TEXT("127.0.0.1");
    PvdPort = 5425;
    PvdTimeoutMilliseconds = 10;
    bAutoStepSimulation = true;

    // Initialize PxSceneFlag options
    bEnableActiveActors = false;
    bEnableCCD = false;
    bDisableCCDResweep = false;
    bEnablePCM = true;  // This is typically enabled by default in PhysX
    bDisableContactReportBufferResize = false;
    bDisableContactCache = false;
    bRequireRWLock = false;
    bEnableStabilization = false;
    bEnableAveragePoint = false;
    bExcludeKinematicsFromActiveActors = false;
    bEnableGPUDynamics = false;
    bEnableEnhancedDeterminism = false;
    bEnableFrictionEveryIteration = false;
    bEnableDirectGPUAPI = false;

    // Initialize other PxSceneDesc options
    Gravity = FVector(0.0f, 0.0f, -9.81f);  // Typical default gravity
    BroadPhaseType = EPxBroadPhaseType::ePABP;
    FrictionType = EPxFrictionType::ePATCH;
    SolverType = EPxSolverType::ePGS;

    // These values should match PxSceneDesc defaults for the default tolerances scale.
    BounceThresholdVelocity = 0.2f;
    FrictionOffsetThreshold = 0.04f;
    FrictionCorrelationDistance = 0.025f;
    SolverBatchSize = 128;
    SolverArticulationBatchSize = 16;
    NbContactDataBlocks = 0;
    MaxNbContactDataBlocks = 1 << 16;
    MaxBiasCoefficient = DefaultPhysXMaxFloat;
    ContactReportStreamBufferSize = 8192;
    CcdMaxPasses = 1;
    CcdThreshold = DefaultPhysXMaxFloat;
    CcdMaxSeparation = 0.04f;
    WakeCounterResetValue = 20.0f * 0.02f;

    // Set default sanity bounds
    SanityBoundsMin = FVector(-DefaultPhysXMaxBoundsExtents);
    SanityBoundsMax = FVector(DefaultPhysXMaxBoundsExtents);

    GPUMaxNumPartitions = 8;
    GPUMaxNumStaticPartitions = 16;
    GPUComputeVersion = 0;
    ContactPairSlabSize = 256;

    // Initialize PhysX pointers
    mFoundation = nullptr;
    mPhysics = nullptr;
    mScene = nullptr;
    mPvd = nullptr;
    mPvdTransport = nullptr;
    mDispatcher = nullptr;
    mDefaultMaterial = nullptr;
}

void UPhysXScene::BeginPlay()
{
    Super::BeginPlay();
    InitializePhysXSimulation();
}

void UPhysXScene::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    ShutdownPhysXSimulation();
}


void UPhysXScene::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (bAutoStepSimulation)
    {
        //StepPhysXSimulation(DeltaTime);
        //StepPhysXSimulation(1.0f / 60.0f);
        StepPhysXSimulation(1.0f / 30.0f);
    }
}

void UPhysXScene::StepPhysXSimulation(float DeltaTime)
{
    if (StepPhysXRuntime(DeltaTime))
    {
        SyncPhysXTransforms();
    }
}

void UPhysXScene::InitializePhysXSimulation()
{
    UPhysXPvdComponent* PvdSettings = ResolvePvdSettings();
    if (!InitializePhysXRuntime(PvdSettings))
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World || !mPhysics || !mScene)
    {
        UE_LOG(LogPhysXPlugin, Error, TEXT("PhysX or World not initialized in UPhysXScene::InitializePhysXActors"));
        return;
    }

    PhysXActors.Empty();

    TArray<AActor*> FoundActors;
    GetAllWorldActors(World, FoundActors);
    TArray<UPhysXJointBase*> PendingJoints;

    for (AActor* Actor : FoundActors)
    {
        if (!Actor)
        {
            continue;
        }

        UPhysXRigidBody* RigidBodyComponent = Actor->FindComponentByClass<UPhysXRigidBody>();
        
        TArray<UBasePhysXCollider*> Colliders;
        Actor->GetComponents<UBasePhysXCollider>(Colliders);

        if (RigidBodyComponent)
        {
            RigidBodyComponent->CreateRigidBody(mPhysics);
            physx::PxRigidActor* RigidActor = RigidBodyComponent->GetPhysXRigidActor();

            for (UBasePhysXCollider* Collider : Colliders)
            {
                Collider->InitializeCollider(mPhysics, RigidActor, mDefaultMaterial);
            }

            RigidBodyComponent->FinalizeRigidBody();

            if (RigidActor)
            {
                AddRigidActorToScene(RigidActor);
                if (RigidBodyComponent->GetPhysXRigidDynamic())
                {
                    PhysXActors.Add(Actor);
                }
            }
        }
        else if (Colliders.Num() > 0)
        { 
            physx::PxRigidActor* StaticActor = CreateStaticRigidActor(Colliders[0]->MakePhysXTransform());
            if (StaticActor)
            {
                for (UBasePhysXCollider* Collider : Colliders)
                {
                    Collider->InitializeCollider(mPhysics, StaticActor, mDefaultMaterial);
                }

                AddRigidActorToScene(StaticActor);
            }
        }

        TArray<UPhysXJointBase*> Joints;
        Actor->GetComponents<UPhysXJointBase>(Joints);
        PendingJoints.Append(Joints);
    }

    for (UPhysXJointBase* Joint : PendingJoints)
    {
        if (Joint)
        {
            Joint->CreateJoint(mPhysics);
        }
    }

    UE_LOG(LogPhysXPlugin, Log, TEXT("Initialized PhysX scene with %d simulated actors and %d joints"), PhysXActors.Num(), PendingJoints.Num());
}

void UPhysXScene::ShutdownPhysXSimulation()
{
    ShutdownPhysXRuntime();
    PhysXActors.Empty();
}

void UPhysXScene::SyncPhysXTransforms()
{
    if (!mScene)
    {
        UE_LOG(LogTemp, Error, TEXT("PhysX scene not initialized in UPhysXScene::SyncPhysXTransforms"));
        return;
    }

    for (AActor* Actor : PhysXActors)
    {
        if (Actor)
        {
            UPhysXRigidBody* RigidBodyComponent = Actor->FindComponentByClass<UPhysXRigidBody>();
            if (RigidBodyComponent)
            {
                RigidBodyComponent->SyncTransformFromPhysX();
            }
        }
    }
}

UPhysXScene* UPhysXScene::FindPhysXScene(const UObject* WorldContextObject)
{
    const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
    if (!World)
    {
        return nullptr;
    }

    TArray<AActor*> FoundActors;
    GetAllWorldActors(World, FoundActors);
    for (AActor* Actor : FoundActors)
    {
        if (UPhysXScene* Scene = Actor ? Actor->FindComponentByClass<UPhysXScene>() : nullptr)
        {
            return Scene;
        }
    }

    return nullptr;
}

UPhysXPvdComponent* UPhysXScene::ResolvePvdSettings() const
{
    return GetOwner() ? GetOwner()->FindComponentByClass<UPhysXPvdComponent>() : nullptr;
}
