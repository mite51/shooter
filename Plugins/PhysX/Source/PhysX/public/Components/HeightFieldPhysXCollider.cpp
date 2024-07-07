#include "HeightFieldPhysXCollider.h"
#include "PhysXPublic.h"
#include "cooking/PxCooking.h"
#include "LandscapeComponent.h"
#include "LandscapeDataAccess.h"

UHeightFieldPhysXCollider::UHeightFieldPhysXCollider()
{
    HeightScale = 1.0f;
    RowScale = 1.0f;
    ColumnScale = 1.0f;
}

physx::PxGeometry* UHeightFieldPhysXCollider::CreateGeometry(physx::PxPhysics* PxPhysics)
{
    if (!LandscapeComponent)
        return nullptr;

    physx::PxHeightField* HeightField = CreatePxHeightField(PxPhysics);
    if (!HeightField)
        return nullptr;

    return new physx::PxHeightFieldGeometry(HeightField, physx::PxMeshGeometryFlags(), HeightScale, RowScale, ColumnScale);
}

physx::PxHeightField* UHeightFieldPhysXCollider::CreatePxHeightField(physx::PxPhysics* PxPhysics)
{
    if (!LandscapeComponent)
        return nullptr;

    FLandscapeComponentDataInterface DataInterface(LandscapeComponent);

    int32 ComponentSizeVerts = DataInterface.GetComponentSizeVerts();
    int32 ComponentSizeQuads = ComponentSizeVerts - 1;

    // Create PxHeightFieldDesc
    physx::PxHeightFieldDesc HFDesc;
    HFDesc.format = physx::PxHeightFieldFormat::eS16_TM;
    HFDesc.nbColumns = ComponentSizeVerts;
    HFDesc.nbRows = ComponentSizeVerts;

    // Allocate memory for samples
    TArray<physx::PxHeightFieldSample> Samples;
    Samples.SetNum(ComponentSizeVerts * ComponentSizeVerts);

    // Fill the samples
    for (int32 Y = 0; Y < ComponentSizeVerts; Y++)
    {
        for (int32 X = 0; X < ComponentSizeVerts; X++)
        {
            int32 SampleIndex = Y * ComponentSizeVerts + X;
            physx::PxHeightFieldSample& Sample = Samples[SampleIndex];

            // Get height data
            uint16 Height = DataInterface.GetHeight(X, Y);
            Sample.height = Height;

            // Set material index (you may want to adjust this based on your needs)
            Sample.materialIndex0.clearBit();
            Sample.materialIndex1.clearBit();

            /*
            // Set tessellation flag (you may want to adjust this based on your needs)
            // For example, let's set it for every other quad in a checkerboard pattern
            if ((X + Y) % 2 == 0)
            {
                Sample.materialIndex0.setBit();
            }
            else
            {
                Sample.materialIndex0.clearBit();
            }
            */
            Sample.materialIndex0.setBit();
        }
    }

    HFDesc.samples.data = Samples.GetData();
    HFDesc.samples.stride = sizeof(physx::PxHeightFieldSample);

    // Create PxHeightField
    physx::PxTolerancesScale Scale;
    physx::PxCookingParams Params(Scale);
    return PxCreateHeightField(HFDesc, PxPhysics->getPhysicsInsertionCallback());
}