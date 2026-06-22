#pragma once

#include "PhysXJointBase.h"

// Temporarily undefine Unreal's check macro to avoid conflicts with PhysX
#pragma push_macro("check")
#undef check

#include "PxPhysicsAPI.h"

// Restore Unreal's check macro
#pragma pop_macro("check")

#include "PhysXDistanceJoint.generated.h"

using namespace physx;

UCLASS(ClassGroup = (PhysX), meta = (BlueprintSpawnableComponent))
class PHYSX_API UPhysXDistanceJoint : public UPhysXJointBase
{
    GENERATED_BODY()

public:
    UPhysXDistanceJoint();

    // Distance joint properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DistanceJoint")
    float MinDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DistanceJoint")
    float MaxDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DistanceJoint")
    float Tolerance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DistanceJoint")
    float Stiffness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DistanceJoint")
    float Damping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DistanceJoint Flags")
    bool MaxDistanceEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DistanceJoint Flags")
    bool MinDistanceEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DistanceJoint Flags")
    bool SpringEnabled;

    // Visualization properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DistanceJoint|Visualization")
    bool bVisualizeJoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DistanceJoint|Visualization")
    float VisualizationScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DistanceJoint|Visualization")
    FColor JointColor;

    // Override functions
    virtual void CreateJoint(PxPhysics* PxPhysics) override;
    virtual void ReleaseJoint() override;

    // Getter functions
    UFUNCTION(BlueprintCallable, Category = "DistanceJoint")
    float GetDistance() const;

    // Setter functions
    UFUNCTION(BlueprintCallable, Category = "DistanceJoint")
    void SetMinDistance(float distance);

    UFUNCTION(BlueprintCallable, Category = "DistanceJoint")
    void SetMaxDistance(float distance);

    UFUNCTION(BlueprintCallable, Category = "DistanceJoint")
    void SetTolerance(float _tolerance);

    UFUNCTION(BlueprintCallable, Category = "DistanceJoint")
    void SetStiffness(float _stiffness);

    UFUNCTION(BlueprintCallable, Category = "DistanceJoint")
    void SetDamping(float _damping);

    UFUNCTION(BlueprintCallable, Category = "DistanceJoint")
    void SetMaxDistanceEnabled(bool value);

    UFUNCTION(BlueprintCallable, Category = "DistanceJoint")
    void SetMinDistanceEnabled(bool value);

    UFUNCTION(BlueprintCallable, Category = "DistanceJoint")
    void SetSpringEnabled(bool v);

private:
    PxDistanceJoint* DistanceJoint;
};