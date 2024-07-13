#include "PhysXScene.h"


#include "../../include/PxPhysicsAPI.h"
#include "BasePhysXCollider.h"
#include "PhysXRigidBody.h"
#include "Kismet/GameplayStatics.h"
#include <vector>

using namespace physx;

#include "foundation/PxErrorCallback.h"
#include "PxPhysXConfig.h"
class PxDefaultErrorCallback : public PxErrorCallback
{
public:
    PxDefaultErrorCallback() {};
    virtual	~PxDefaultErrorCallback() {};

    virtual void reportError(PxErrorCode::Enum code, const char* message, const char* file, int line) PX_OVERRIDE 
    {
        UE_LOG(LogTemp, Error, TEXT("PxErrorCallback %d %S %S %d"), (int)code, message, file, line);
    };
};

#include "foundation/PxAllocatorCallback.h"
#include "foundation/PxAssert.h"
#include "foundation/PxMemory.h"
#include "common/PxPhysXCommonConfig.h"
#if PX_WINDOWS_FAMILY || PX_LINUX_FAMILY || PX_SWITCH
#include <malloc.h>
#endif
class PxDefaultAllocator : public PxAllocatorCallback
{
public:
    virtual void* allocate(size_t size, const char*, const char*, int)
    {
        //UE_LOG(LogTemp, Error, TEXT("PxDefaultAllocator %d"), (int)size );
        void* ptr = platformAlignedAlloc(size);
        PX_ASSERT((size_t(ptr) & 15) == 0);
#if PX_STOMP_ALLOCATED_MEMORY
        if (ptr != NULL)
        {
            PxMemSet(ptr, PxI32(0xcd), PxU32(size));
        }
#endif
        return ptr;
    }

    virtual void deallocate(void* ptr)
    {
        platformAlignedFree(ptr);
    }
};


static ::PxDefaultErrorCallback	gErrorCallback;
static ::PxDefaultAllocator		gAllocator;

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
    if (mScene)
    {
        mScene->simulate(DeltaTime);
        mScene->fetchResults(true);
        SyncPhysXTransforms();
    }
}

void UPhysXScene::InitializePhysXSimulation()
{
    if(mFoundation == nullptr)
    {
        mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
    }

    if (!mFoundation)
    {
        UE_LOG(LogTemp, Error, TEXT("PxCreateFoundation failed!"));
        return;
    }

    mPvd = PxCreatePvd(*mFoundation);
    physx::PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(TCHAR_TO_ANSI(*PVD_HOST_IP), 5425, 10);
    mPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

    mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, physx::PxTolerancesScale(), true, mPvd);
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
    UWorld* World = GetWorld();
    if (!World || !mPhysics || !mScene)
    {
        UE_LOG(LogTemp, Error, TEXT("PhysX or World not initialized in UPhysXScene::InitializePhysXActors"));
        return;
    }

    // Clear the existing array of PhysX actors
    PhysXActors.Empty();

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), FoundActors);

    // Find all actors with UPhysXRigidBody component
    for (AActor* Actor : FoundActors)
    {
        UPhysXRigidBody* RigidBodyComponent = Actor->FindComponentByClass<UPhysXRigidBody>();
        
        // Find and initialize all colliders
        TArray<UBasePhysXCollider*> Colliders;
        Actor->GetComponents<UBasePhysXCollider>(Colliders);

        if (RigidBodyComponent)
        {

            // Create the PhysX rigid body
            RigidBodyComponent->CreateRigidBody(mPhysics);

            physx::PxRigidDynamic* pDynamic = RigidBodyComponent->RigidBody;

            for (UBasePhysXCollider* Collider : Colliders)
            {
                Collider->InitializeCollider(mPhysics, pDynamic);
            }

            //PxRigidBodyExt::updateMassAndInertia(*pDynamic, 10.0f);

            // Add the rigid body to the PhysX scene
            mScene->addActor(*pDynamic);

            // Add the actor to our PhysX actors array
            PhysXActors.Add(Actor);
        }
        else if (Colliders.Num() > 0)
        { 
            // Create the PhysX static body
            // NOTE : I'm not sure this is the right way to build static actors :/
            // a plane seems to need a special transform, but then how can one 
            // static actor have mulitple shapes that need different tranforms?
            physx::PxTransform T = Colliders[0]->MakePhysXTransform();
            physx::PxRigidStatic* pStatic = mPhysics->createRigidStatic(T);

            for (UBasePhysXCollider* Collider : Colliders)
            {
                //Collider->InitializeCollider(mPhysics, pStatic);
                physx::PxGeometry* Geometry = Collider->CreateGeometry(mPhysics);
                physx::PxShape* ColliderShape = mPhysics->createShape(*Geometry, *mPhysics->createMaterial(0.5f, 0.5f, 0.1f), true);
                pStatic->attachShape(*ColliderShape);
                ColliderShape->release();
            }

            // Add the rigid body to the PhysX scene
            mScene->addActor(*pStatic);

        }
    }

    UE_LOG(LogTemp, Log, TEXT("Initialized %d PhysX scene"), PhysXActors.Num());
}

void UPhysXScene::ShutdownPhysXSimulation()
{
    PX_RELEASE(mScene);
    PX_RELEASE(mDispatcher);
    PX_RELEASE(mPhysics);
    if (mPvd)
    {
        PxPvdTransport* transport = mPvd->getTransport();
        PX_RELEASE(mPvd);
        PX_RELEASE(transport);
    }
    PX_RELEASE(mFoundation);

    mScene = nullptr;
    mPhysics = nullptr;
    mFoundation = nullptr;
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

constexpr uint32 FlagToShift(physx::PxSceneFlag::Enum flag)
{
    return static_cast<uint32>(flag) >= 32 ? 0 : static_cast<uint32>(flag);
}

physx::PxSceneDesc UPhysXScene::CreateSceneDesc(physx::PxPhysics* Physics)
{
    physx::PxSceneDesc sceneDesc(Physics->getTolerancesScale());

    mDispatcher = PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = mDispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;

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