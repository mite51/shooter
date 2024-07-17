#pragma once

#include "PhysXJointBase.h"
#include "PhysXD6Joint.generated.h"

using namespace physx;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PHYSX_API UPhysXD6Joint : public UPhysXJointBase
{
    GENERATED_BODY()

public:
    UPhysXD6Joint();

    // Motion types for each axis
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "D6Joint|Motion")
    EPxD6Motion MotionX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "D6Joint|Motion")
    EPxD6Motion MotionY;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "D6Joint|Motion")
    EPxD6Motion MotionZ;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "D6Joint|Motion")
    EPxD6Motion MotionTwist;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "D6Joint|Motion")
    EPxD6Motion MotionSwing1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "D6Joint|Motion")
    EPxD6Motion MotionSwing2;

    // Linear limits
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "D6Joint|Limits")
    float DistanceLimit;

    // Angular limits
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "D6Joint|Limits")
    FVector2D TwistLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "D6Joint|Limits")
    float SwingLimitY;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "D6Joint|Limits")
    float SwingLimitZ;

    // Drive properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "D6Joint|Drive")
    FPxD6JointDrive DriveX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "D6Joint|Drive")
    FPxD6JointDrive DriveY;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "D6Joint|Drive")
    FPxD6JointDrive DriveZ;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "D6Joint|Drive")
    FPxD6JointDrive DriveSwing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "D6Joint|Drive")
    FPxD6JointDrive DriveTwist;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "D6Joint|Drive")
    FPxD6JointDrive DriveSlerp;

    // Drive targets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "D6Joint|Drive")
    FTransform DrivePosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "D6Joint|Drive")
    FVector DriveLinearVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "D6Joint|Drive")
    FVector DriveAngularVelocity;

    // Override functions
    virtual void CreateJoint(physx::PxPhysics* PxPhysics) override;
    virtual void ReleaseJoint() override;

    // Getter functions
    UFUNCTION(BlueprintCallable, Category = "D6Joint")
    float GetTwistAngle() const;

    UFUNCTION(BlueprintCallable, Category = "D6Joint")
    float GetSwingYAngle() const;

    UFUNCTION(BlueprintCallable, Category = "D6Joint")
    float GetSwingZAngle() const;

    // Setter functions
    UFUNCTION(BlueprintCallable, Category = "D6Joint")
    void SetMotion(EPxD6Axis Axis, EPxD6Motion Motion);

    UFUNCTION(BlueprintCallable, Category = "D6Joint")
    void SetDistanceLimit(float Limit);

    UFUNCTION(BlueprintCallable, Category = "D6Joint")
    void SetLinearLimit(EPxD6Axis Axis, const FVector2D& Limit);

    UFUNCTION(BlueprintCallable, Category = "D6Joint")
    void SetTwistLimit(const FVector2D& Limit);

    UFUNCTION(BlueprintCallable, Category = "D6Joint")
    void SetSwingLimit(float YLimit, float ZLimit);

    UFUNCTION(BlueprintCallable, Category = "D6Joint")
    void SetDrive(EPxD6Drive DriveType, const FPxD6JointDrive& Drive);

    UFUNCTION(BlueprintCallable, Category = "D6Joint")
    void SetDrivePosition(const FTransform& Pose, bool bAutowake = true);

    UFUNCTION(BlueprintCallable, Category = "D6Joint")
    void SetDriveVelocity(const FVector& Linear, const FVector& Angular, bool bAutowake = true);

    // visualization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "D6Joint|Visualization")
    bool bVisualizeJoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "D6Joint|Visualization")
    float VisualizationScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "D6Joint|Visualization")
    FColor LinearLimitColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "D6Joint|Visualization")
    FColor SwingLimitColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "D6Joint|Visualization")
    FColor TwistLimitColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "D6Joint|Visualization")
    FColor DriveColor;

private:
    PxD6Joint* D6Joint;
};
