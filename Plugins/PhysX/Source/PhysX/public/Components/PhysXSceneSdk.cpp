#include "PhysXScene.h"

#include "Components/PhysXPvdComponent.h"
#include "PhysX.h"
#include "NvidiaPhysXIncludes.h"
#include "PhysXTypeConversions.h"
#include "extensions/PxDefaultCpuDispatcher.h"
#include "extensions/PxDefaultSimulationFilterShader.h"
#include "pvd/PxPvdTransport.h"

namespace
{
    physx::PxDefaultErrorCallback GPhysXErrorCallback;
    physx::PxDefaultAllocator GPhysXAllocator;

    physx::PxBroadPhaseType::Enum ToPxBroadPhaseType(EPxBroadPhaseType Type)
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

    physx::PxFrictionType::Enum ToPxFrictionType(EPxFrictionType Type)
    {
        switch (Type)
        {
        case EPxFrictionType::ePATCH:
        case EPxFrictionType::eTWO_DIRECTIONAL:
            return physx::PxFrictionType::ePATCH;
        default:
            return physx::PxFrictionType::ePATCH;
        }
    }

    physx::PxSolverType::Enum ToPxSolverType(EPxSolverType Type)
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
}

bool UPhysXScene::InitializePhysXRuntime(UPhysXPvdComponent* PvdSettings)
{
    if (mScene && mPhysics)
    {
        return true;
    }

    if (mFoundation == nullptr)
    {
        mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, GPhysXAllocator, GPhysXErrorCallback);
    }

    if (!mFoundation)
    {
        UE_LOG(LogPhysXPlugin, Error, TEXT("PxCreateFoundation failed"));
        return false;
    }

    const bool bWantsPvd = PvdSettings ? PvdSettings->bConnectToPvd : bEnablePvd;
    if (bWantsPvd)
    {
        const FString Host = PvdSettings ? PvdSettings->Host : PVD_HOST_IP;
        const int32 Port = PvdSettings ? PvdSettings->Port : PvdPort;
        const int32 TimeoutMs = PvdSettings ? PvdSettings->TimeoutMilliseconds : PvdTimeoutMilliseconds;

        mPvd = PxCreatePvd(*mFoundation);
        mPvdTransport = physx::PxDefaultPvdSocketTransportCreate(TCHAR_TO_ANSI(*Host), Port, TimeoutMs);
        if (mPvd && mPvdTransport)
        {
            const physx::PxPvdInstrumentationFlags Flags = PvdSettings
                ? physx::PxPvdInstrumentationFlags(static_cast<physx::PxPvdInstrumentationFlag::Enum>(PvdSettings->MakeInstrumentationFlagsMask()))
                : physx::PxPvdInstrumentationFlag::eALL;
            mPvd->connect(*mPvdTransport, Flags);
        }
    }

    mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, physx::PxTolerancesScale(), mPvd != nullptr, mPvd);
    if (!mPhysics)
    {
        UE_LOG(LogPhysXPlugin, Error, TEXT("PxCreatePhysics failed"));
        return false;
    }

    mDefaultMaterial = mPhysics->createMaterial(0.5f, 0.5f, 0.1f);

    physx::PxSceneDesc SceneDesc = CreateSceneDesc(mPhysics);
    mScene = mPhysics->createScene(SceneDesc);
    if (!mScene)
    {
        UE_LOG(LogPhysXPlugin, Error, TEXT("createScene failed"));
        return false;
    }

    ConfigurePvdSceneClient(PvdSettings);
    return true;
}

bool UPhysXScene::StepPhysXRuntime(float DeltaTime)
{
    if (!mScene)
    {
        return false;
    }

    mScene->simulate(DeltaTime);
    mScene->fetchResults(true);
    return true;
}

void UPhysXScene::ShutdownPhysXRuntime()
{
    PX_RELEASE(mScene);
    PX_RELEASE(mDispatcher);
    PX_RELEASE(mDefaultMaterial);
    PX_RELEASE(mPhysics);
    ReleasePvd();
    PX_RELEASE(mFoundation);

    mScene = nullptr;
    mPhysics = nullptr;
    mFoundation = nullptr;
    mDefaultMaterial = nullptr;
}

physx::PxRigidActor* UPhysXScene::CreateStaticRigidActor(const FTransform& Transform) const
{
    return mPhysics ? mPhysics->createRigidStatic(UEPhysX::ToPxTransform(Transform)) : nullptr;
}

void UPhysXScene::AddRigidActorToScene(physx::PxRigidActor* RigidActor) const
{
    if (mScene && RigidActor)
    {
        mScene->addActor(*RigidActor);
    }
}

void UPhysXScene::ConfigurePvdSceneClient(UPhysXPvdComponent* PvdSettings) const
{
    if (!mScene || !PvdSettings)
    {
        return;
    }

    physx::PxPvdSceneClient* PvdClient = mScene->getScenePvdClient();
    if (!PvdClient)
    {
        return;
    }

    PvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, PvdSettings->bTransmitConstraints);
    PvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, PvdSettings->bTransmitContacts);
    PvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, PvdSettings->bTransmitSceneQueries);
}

void UPhysXScene::ReleasePvd()
{
    PX_RELEASE(mPvd);
    PX_RELEASE(mPvdTransport);
    mPvd = nullptr;
    mPvdTransport = nullptr;
}

physx::PxSceneDesc UPhysXScene::CreateSceneDesc(physx::PxPhysics* Physics)
{
    physx::PxSceneDesc SceneDesc(Physics->getTolerancesScale());

    mDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
    SceneDesc.cpuDispatcher = mDispatcher;
    SceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
    SceneDesc.gravity = physx::PxVec3(Gravity.X, Gravity.Y, Gravity.Z);

    SceneDesc.flags = physx::PxSceneFlags(0);
    if (bEnableActiveActors) SceneDesc.flags |= physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS;
    if (bEnableCCD) SceneDesc.flags |= physx::PxSceneFlag::eENABLE_CCD;
    if (bDisableCCDResweep) SceneDesc.flags |= physx::PxSceneFlag::eDISABLE_CCD_RESWEEP;
    if (bEnablePCM) SceneDesc.flags |= physx::PxSceneFlag::eENABLE_PCM;
    if (bDisableContactReportBufferResize) SceneDesc.flags |= physx::PxSceneFlag::eDISABLE_CONTACT_REPORT_BUFFER_RESIZE;
    if (bDisableContactCache) SceneDesc.flags |= physx::PxSceneFlag::eDISABLE_CONTACT_CACHE;
    if (bRequireRWLock) SceneDesc.flags |= physx::PxSceneFlag::eREQUIRE_RW_LOCK;
    if (bEnableStabilization) SceneDesc.flags |= physx::PxSceneFlag::eENABLE_STABILIZATION;
    if (bEnableAveragePoint) SceneDesc.flags |= physx::PxSceneFlag::eENABLE_AVERAGE_POINT;
    if (bExcludeKinematicsFromActiveActors) SceneDesc.flags |= physx::PxSceneFlag::eEXCLUDE_KINEMATICS_FROM_ACTIVE_ACTORS;
    if (bEnableGPUDynamics) SceneDesc.flags |= physx::PxSceneFlag::eENABLE_GPU_DYNAMICS;
    if (bEnableEnhancedDeterminism) SceneDesc.flags |= physx::PxSceneFlag::eENABLE_ENHANCED_DETERMINISM;
    if (bEnableFrictionEveryIteration) SceneDesc.flags |= physx::PxSceneFlag::eENABLE_FRICTION_EVERY_ITERATION;
    if (bEnableDirectGPUAPI) SceneDesc.flags |= physx::PxSceneFlag::eENABLE_DIRECT_GPU_API;

    SceneDesc.broadPhaseType = ToPxBroadPhaseType(BroadPhaseType);
    SceneDesc.frictionType = ToPxFrictionType(FrictionType);
    SceneDesc.solverType = ToPxSolverType(SolverType);
    SceneDesc.bounceThresholdVelocity = BounceThresholdVelocity;
    SceneDesc.frictionOffsetThreshold = FrictionOffsetThreshold;
    SceneDesc.frictionCorrelationDistance = FrictionCorrelationDistance;
    SceneDesc.solverBatchSize = SolverBatchSize;
    SceneDesc.solverArticulationBatchSize = SolverArticulationBatchSize;
    SceneDesc.nbContactDataBlocks = NbContactDataBlocks;
    SceneDesc.maxNbContactDataBlocks = MaxNbContactDataBlocks;
    SceneDesc.maxBiasCoefficient = MaxBiasCoefficient;
    SceneDesc.contactReportStreamBufferSize = ContactReportStreamBufferSize;
    SceneDesc.ccdMaxPasses = CcdMaxPasses;
    SceneDesc.ccdThreshold = CcdThreshold;
    SceneDesc.ccdMaxSeparation = CcdMaxSeparation;
    SceneDesc.wakeCounterResetValue = WakeCounterResetValue;
    SceneDesc.sanityBounds = physx::PxBounds3(
        physx::PxVec3(SanityBoundsMin.X, SanityBoundsMin.Y, SanityBoundsMin.Z),
        physx::PxVec3(SanityBoundsMax.X, SanityBoundsMax.Y, SanityBoundsMax.Z)
    );
    SceneDesc.gpuMaxNumPartitions = GPUMaxNumPartitions;
    SceneDesc.gpuMaxNumStaticPartitions = GPUMaxNumStaticPartitions;
    SceneDesc.contactPairSlabSize = ContactPairSlabSize;

    return SceneDesc;
}
