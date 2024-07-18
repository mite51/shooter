#pragma once

#include "PhysXDistanceJoint.h"
#include "ComponentVisualizer.h"

class PhysXDistanceJoint_Visualizer : public FComponentVisualizer
{
public:
    // Override to draw visualizations
    virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;

private:
    void DrawDistanceJoint(const UPhysXDistanceJoint* Joint, const FSceneView* View, FPrimitiveDrawInterface* PDI);
};