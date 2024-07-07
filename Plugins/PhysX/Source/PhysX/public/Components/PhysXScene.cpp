#include "PhysXScene.h"


#include "../../include/PxPhysicsAPI.h"

using namespace physx;
static PxDefaultErrorCallback	gErrorCallback;
static PxDefaultAllocator		gAllocator;

UPhysXScene::UPhysXScene()
{
    PrimaryComponentTick.bCanEverTick = true;

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

    // These values should match PxSceneDesc defaults
    BounceThresholdVelocity = 0.2f * PxTolerancesScale().speed;
    FrictionOffsetThreshold = 0.04f * PxTolerancesScale().length;
    FrictionCorrelationDistance = 0.025f * PxTolerancesScale().length;
    SolverBatchSize = 128;
    SolverArticulationBatchSize = 16;
    NbContactDataBlocks = 0;
    MaxNbContactDataBlocks = 1 << 16;
    MaxBiasCoefficient = PX_MAX_F32;
    ContactReportStreamBufferSize = 8192;
    CcdMaxPasses = 1;
    CcdThreshold = PX_MAX_F32;
    CcdMaxSeparation = 0.04f * PxTolerancesScale().length;
    WakeCounterResetValue = 20.0f * 0.02f;

    // Set default sanity bounds
    SanityBoundsMin = FVector(-PX_MAX_BOUNDS_EXTENTS);
    SanityBoundsMax = FVector(PX_MAX_BOUNDS_EXTENTS);

    GPUMaxNumPartitions = 8;
    GPUMaxNumStaticPartitions = 16;
    GPUComputeVersion = 0;
    ContactPairSlabSize = 256;

    // Initialize PhysX pointers
    mFoundation = nullptr;
    mPhysics = nullptr;
    mScene = nullptr;
}

void UPhysXScene::BeginPlay()
{
    Super::BeginPlay();
    InitializePhysXSimulation();
}

void UPhysXScene::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    StepPhysXSimulation(DeltaTime);
}

void UPhysXScene::StepPhysXSimulation(float DeltaTime)
{
    if (mScene)
    {
        mScene->simulate(DeltaTime);
        mScene->fetchResults(true);
    }
}

void UPhysXScene::InitializePhysXSimulation()
{
    mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
    if (!mFoundation)
    {
        UE_LOG(LogTemp, Error, TEXT("PxCreateFoundation failed!"));
        return;
    }

    mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, physx::PxTolerancesScale());
    if (!mPhysics)
    {
        UE_LOG(LogTemp, Error, TEXT("PxCreatePhysics failed!"));
        return;
    }

    physx::PxSceneDesc sceneDesc = CreateSceneDesc(mPhysics);
    mScene = mPhysics->createScene(sceneDesc);
    if (!mScene)
    {
        UE_LOG(LogTemp, Error, TEXT("createScene failed!"));
        return;
    }

    // Additional setup code here...
}

constexpr uint32 FlagToShift(physx::PxSceneFlag::Enum flag)
{
    return static_cast<uint32>(flag) >= 32 ? 0 : static_cast<uint32>(flag);
}

physx::PxSceneDesc UPhysXScene::CreateSceneDesc(physx::PxPhysics* Physics)
{
    physx::PxSceneDesc sceneDesc(Physics->getTolerancesScale());

    // Set gravity
    sceneDesc.gravity = physx::PxVec3(Gravity.X, Gravity.Y, Gravity.Z);

    // Set scene flags
    sceneDesc.flags = physx::PxSceneFlags(0);
    if (bEnableActiveActors) sceneDesc.flags |= physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS;
    if (bEnableCCD) sceneDesc.flags |= physx::PxSceneFlag::eENABLE_CCD;
    if (bDisableCCDResweep) sceneDesc.flags |= physx::PxSceneFlag::eDISABLE_CCD_RESWEEP;
    if (bEnablePCM) sceneDesc.flags |= physx::PxSceneFlag::eENABLE_PCM;
    if (bDisableContactReportBufferResize) sceneDesc.flags |= physx::PxSceneFlag::eDISABLE_CONTACT_REPORT_BUFFER_RESIZE;
    if (bDisableContactCache) sceneDesc.flags |= physx::PxSceneFlag::eDISABLE_CONTACT_CACHE;
    if (bRequireRWLock) sceneDesc.flags |= physx::PxSceneFlag::eREQUIRE_RW_LOCK;
    if (bEnableStabilization) sceneDesc.flags |= physx::PxSceneFlag::eENABLE_STABILIZATION;
    if (bEnableAveragePoint) sceneDesc.flags |= physx::PxSceneFlag::eENABLE_AVERAGE_POINT;
    if (bExcludeKinematicsFromActiveActors) sceneDesc.flags |= physx::PxSceneFlag::eEXCLUDE_KINEMATICS_FROM_ACTIVE_ACTORS;
    if (bEnableGPUDynamics) sceneDesc.flags |= physx::PxSceneFlag::eENABLE_GPU_DYNAMICS;
    if (bEnableEnhancedDeterminism) sceneDesc.flags |= physx::PxSceneFlag::eENABLE_ENHANCED_DETERMINISM;
    if (bEnableFrictionEveryIteration) sceneDesc.flags |= physx::PxSceneFlag::eENABLE_FRICTION_EVERY_ITERATION;
    if (bEnableDirectGPUAPI) sceneDesc.flags |= physx::PxSceneFlag::eENABLE_DIRECT_GPU_API;

    // Set other PxSceneDesc options
    sceneDesc.broadPhaseType = ToPxBroadPhaseType(BroadPhaseType);
    sceneDesc.frictionType = ToPxFrictionType(FrictionType);
    sceneDesc.solverType = ToPxSolverType(SolverType);
    sceneDesc.bounceThresholdVelocity = BounceThresholdVelocity;
    sceneDesc.frictionOffsetThreshold = FrictionOffsetThreshold;
    sceneDesc.frictionCorrelationDistance = FrictionCorrelationDistance;
    sceneDesc.solverBatchSize = SolverBatchSize;
    sceneDesc.solverArticulationBatchSize = SolverArticulationBatchSize;
    sceneDesc.nbContactDataBlocks = NbContactDataBlocks;
    sceneDesc.maxNbContactDataBlocks = MaxNbContactDataBlocks;
    sceneDesc.maxBiasCoefficient = MaxBiasCoefficient;
    sceneDesc.contactReportStreamBufferSize = ContactReportStreamBufferSize;
    sceneDesc.ccdMaxPasses = CcdMaxPasses;
    sceneDesc.ccdThreshold = CcdThreshold;
    sceneDesc.ccdMaxSeparation = CcdMaxSeparation;
    sceneDesc.wakeCounterResetValue = WakeCounterResetValue;
    sceneDesc.sanityBounds = physx::PxBounds3(
        physx::PxVec3(SanityBoundsMin.X, SanityBoundsMin.Y, SanityBoundsMin.Z),
        physx::PxVec3(SanityBoundsMax.X, SanityBoundsMax.Y, SanityBoundsMax.Z)
    );
    sceneDesc.gpuMaxNumPartitions = GPUMaxNumPartitions;
    sceneDesc.gpuMaxNumStaticPartitions = GPUMaxNumStaticPartitions;
    sceneDesc.contactPairSlabSize = ContactPairSlabSize;
/* leave as default
    // Set up GPU dynamics config
    sceneDesc.gpuDynamicsConfig.tempBufferCapacity = gpuDynamicsConfig.tempBufferCapacity;
    sceneDesc.gpuDynamicsConfig.maxRigidContactCount = gpuDynamicsConfig.maxRigidContactCount;
    sceneDesc.gpuDynamicsConfig.maxRigidPatchCount = gpuDynamicsConfig.maxRigidPatchCount;
    sceneDesc.gpuDynamicsConfig.heapCapacity = gpuDynamicsConfig.heapCapacity;
    sceneDesc.gpuDynamicsConfig.foundLostPairsCapacity = gpuDynamicsConfig.foundLostPairsCapacity;
    sceneDesc.gpuDynamicsConfig.foundLostAggregatePairsCapacity = gpuDynamicsConfig.foundLostAggregatePairsCapacity;
    sceneDesc.gpuDynamicsConfig.totalAggregatePairsCapacity = gpuDynamicsConfig.totalAggregatePairsCapacity;
    sceneDesc.gpuDynamicsConfig.maxSoftBodyContacts = gpuDynamicsConfig.maxSoftBodyContacts;
    sceneDesc.gpuDynamicsConfig.maxFemClothContacts = gpuDynamicsConfig.maxFemClothContacts;
    sceneDesc.gpuDynamicsConfig.maxParticleContacts = gpuDynamicsConfig.maxParticleContacts;
    sceneDesc.gpuDynamicsConfig.collisionStackSize = gpuDynamicsConfig.collisionStackSize;
    sceneDesc.gpuDynamicsConfig.maxHairContacts = gpuDynamicsConfig.maxHairContacts;
*/
    return sceneDesc;
}

// Helper functions to convert UE4 enums to PhysX enums
physx::PxBroadPhaseType::Enum UPhysXScene::ToPxBroadPhaseType(EPxBroadPhaseType Type)
{
    switch (Type)
    {
    case EPxBroadPhaseType::eSAP:
        return physx::PxBroadPhaseType::eSAP;
    case EPxBroadPhaseType::eMBP:
        return physx::PxBroadPhaseType::eMBP;
    case EPxBroadPhaseType::ePABP:
        return physx::PxBroadPhaseType::ePABP;
    case EPxBroadPhaseType::eGPU:
        return physx::PxBroadPhaseType::eGPU;
    default:
        return physx::PxBroadPhaseType::ePABP;
    }
}

physx::PxFrictionType::Enum UPhysXScene::ToPxFrictionType(EPxFrictionType Type)
{
    switch (Type)
    {
    case EPxFrictionType::ePATCH:
        return physx::PxFrictionType::ePATCH;
    case EPxFrictionType::eTWO_DIRECTIONAL:
        return physx::PxFrictionType::eTWO_DIRECTIONAL;
    default:
        return physx::PxFrictionType::ePATCH;
    }
}

physx::PxSolverType::Enum UPhysXScene::ToPxSolverType(EPxSolverType Type)
{
    switch (Type)
    {
    case EPxSolverType::ePGS:
        return physx::PxSolverType::ePGS;
    case EPxSolverType::eTGS:
        return physx::PxSolverType::eTGS;
    default:
        return physx::PxSolverType::ePGS;
    }
}