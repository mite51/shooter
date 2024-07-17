#pragma once

#include "PhysXD6Joint.h"
#include "ComponentVisualizer.h"

using namespace physx;

class PhysXD6Joint_Visualizer : public FComponentVisualizer
{

public:
    // Override to draw visualizations
    virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;

private:

    // visualization
    void DrawLinearLimits(const UPhysXD6Joint* joint, const FSceneView* View, FPrimitiveDrawInterface* PDI);
    void DrawAngularLimits(const UPhysXD6Joint* joint, const FSceneView* View, FPrimitiveDrawInterface* PDI);
    void DrawDrives(const UPhysXD6Joint* joint, const FSceneView* View, FPrimitiveDrawInterface* PDI);
};
