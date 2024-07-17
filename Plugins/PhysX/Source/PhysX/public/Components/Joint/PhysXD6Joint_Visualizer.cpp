#include "PhysXD6Joint_Visualizer.h"
#include "CoreMinimal.h"
#include "MathUtil.h"

void PhysXD6Joint_Visualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
    const UPhysXD6Joint* joint = Cast<UPhysXD6Joint>(Component);

    if (!joint->bVisualizeJoint || !joint->Actor0 || !PDI)
    {
        return;
    }


    DrawLinearLimits(joint, View, PDI);
    DrawAngularLimits(joint, View, PDI);
    DrawDrives(joint, View, PDI);

}

void PhysXD6Joint_Visualizer::DrawLinearLimits(const UPhysXD6Joint* joint, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
    // Draw distance limit
    if (joint->MotionX == EPxD6Motion::Limited || joint->MotionY == EPxD6Motion::Limited || joint->MotionZ == EPxD6Motion::Limited)
    {
        FTransform JointTransform = joint->Actor0->GetActorTransform() * joint->LocalPose0;
        DrawWireSphere(PDI, JointTransform.GetLocation(), joint->LinearLimitColor, joint->DistanceLimit, 32, SDPG_Foreground, 1.0f);
    }
}

void PhysXD6Joint_Visualizer::DrawAngularLimits(const UPhysXD6Joint* joint, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
    FTransform JointTransform = joint->Actor0->GetActorTransform() * joint->LocalPose0;

    // Draw twist limit
    if (joint->MotionTwist == EPxD6Motion::Limited)
    {
        FVector base = JointTransform.GetTranslation() + JointTransform.GetUnitAxis(EAxis::X) * joint->VisualizationScale;
        DrawArc(PDI, base, JointTransform.GetUnitAxis(EAxis::Z), JointTransform.GetUnitAxis(EAxis::Y), -joint->TwistLimit.X, joint->TwistLimit.X, joint->VisualizationScale * 0.5f, 32, joint->TwistLimitColor, SDPG_Foreground);

        base = JointTransform.GetTranslation() + JointTransform.GetUnitAxis(EAxis::Z) * joint->VisualizationScale;
        DrawArc(PDI, base, JointTransform.GetUnitAxis(EAxis::X), JointTransform.GetUnitAxis(EAxis::Y), -joint->TwistLimit.Y, joint->TwistLimit.Y, joint->VisualizationScale * 0.5f, 32, joint->TwistLimitColor, SDPG_Foreground);

    }

    // Draw swing limits
    if (joint->MotionSwing1 == EPxD6Motion::Limited || joint->MotionSwing2 == EPxD6Motion::Limited)
    {
        DrawArc(PDI, JointTransform.GetTranslation(), JointTransform.GetUnitAxis(EAxis::X), JointTransform.GetUnitAxis(EAxis::Y), -joint->SwingLimitY, joint->SwingLimitY, joint->VisualizationScale, 32, joint->SwingLimitColor, SDPG_Foreground);
        DrawArc(PDI, JointTransform.GetTranslation(), JointTransform.GetUnitAxis(EAxis::Z), JointTransform.GetUnitAxis(EAxis::X), -joint->SwingLimitZ, joint->SwingLimitZ, joint->VisualizationScale, 32, joint->SwingLimitColor, SDPG_Foreground);
    }
}

void PhysXD6Joint_Visualizer::DrawDrives(const UPhysXD6Joint* joint, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
    FTransform JointTransform = joint->Actor0->GetActorTransform() * joint->LocalPose0;

    // Draw linear drives
    if (joint->DriveX.Stiffness > 0 || joint->DriveY.Stiffness > 0 || joint->DriveZ.Stiffness > 0)
    {
        FVector DriveTarget = JointTransform.TransformPosition(joint->DrivePosition.GetLocation());
        DrawWireSphere(PDI, DriveTarget, joint->DriveColor, joint->VisualizationScale * 0.25f, 32, SDPG_Foreground, 0.5f);
    }

    // Draw angular drives
    if (joint->DriveSwing.Stiffness > 0 || joint->DriveTwist.Stiffness > 0 || joint->DriveSlerp.Stiffness > 0)
    {
        FQuat DriveRotation = JointTransform.GetRotation() * joint->DrivePosition.GetRotation();
        DrawCoordinateSystem(PDI, JointTransform.GetLocation(), DriveRotation.Rotator(), joint->VisualizationScale, SDPG_Foreground, 1.0f);
    }
}