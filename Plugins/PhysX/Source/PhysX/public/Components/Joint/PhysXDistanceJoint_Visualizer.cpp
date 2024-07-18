#include "PhysXDistanceJoint_Visualizer.h"
#include "SceneManagement.h"
#include "DrawDebugHelpers.h"

void PhysXDistanceJoint_Visualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
    const UPhysXDistanceJoint* Joint = Cast<UPhysXDistanceJoint>(Component);
    if (Joint && Joint->bVisualizeJoint)
    {
        DrawDistanceJoint(Joint, View, PDI);
    }
}

void PhysXDistanceJoint_Visualizer::DrawDistanceJoint(const UPhysXDistanceJoint* Joint, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
    if (!Joint->Actor0 || !Joint->Actor1)
    {
        return;
    }

    FTransform Transform0 = Joint->Actor0->GetActorTransform() * Joint->LocalPose0;
    FTransform Transform1 = Joint->Actor1->GetActorTransform() * Joint->LocalPose1;

    FVector Start = Transform0.GetLocation();
    FVector End = Transform1.GetLocation();

    // Draw the main line connecting the two anchor points
    PDI->DrawLine(Start, End, Joint->JointColor, SDPG_Foreground, 2.0f);

    //
    FVector middle = Start + ((End - Start) * 0.5f);

    // Draw min and max distance limits if enabled
    if (Joint->MaxDistanceEnabled)
    {
        DrawWireSphere(PDI, middle, Joint->JointColor, Joint->MaxDistance, 32, SDPG_Foreground);
    }

    if (Joint->MinDistanceEnabled)
    {
        DrawWireSphere(PDI, middle, Joint->JointColor, Joint->MinDistance, 32, SDPG_Foreground);
    }
}