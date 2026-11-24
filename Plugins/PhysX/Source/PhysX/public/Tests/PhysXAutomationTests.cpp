#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Components/PhysXMaterial.h"
#include "Components/PhysXRigidBody.h"
#include "foundation/PxPhysicsVersion.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPhysXSdkVersionTest, "PhysX.Plugin.SDKVersion", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPhysXSdkVersionTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("PhysX major version"), PX_PHYSICS_VERSION_MAJOR, 5);
	TestEqual(TEXT("PhysX minor version"), PX_PHYSICS_VERSION_MINOR, 6);
	TestEqual(TEXT("PhysX bugfix version"), PX_PHYSICS_VERSION_BUGFIX, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPhysXComponentDefaultsTest, "PhysX.Plugin.ComponentDefaults", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPhysXComponentDefaultsTest::RunTest(const FString& Parameters)
{
	UPhysXMaterial* Material = NewObject<UPhysXMaterial>();
	TestNotNull(TEXT("Material component"), Material);
	TestEqual(TEXT("Default static friction"), Material->StaticFriction, 0.5f);
	TestEqual(TEXT("Default dynamic friction"), Material->DynamicFriction, 0.5f);
	TestEqual(TEXT("Default restitution"), Material->Restitution, 0.1f);

	UPhysXRigidBody* Body = NewObject<UPhysXRigidBody>();
	TestNotNull(TEXT("Rigid body component"), Body);
	TestEqual(TEXT("Default body type"), static_cast<uint8>(Body->BodyType), static_cast<uint8>(EPhysXRigidBodyType::Dynamic));
	TestFalse(TEXT("Default rigid body is not kinematic"), Body->KINEMATIC);

	return true;
}

#if WITH_EDITOR

#include "Editor.h"
#include "Import/PhysXUsdImportLibrary.h"
#include "Misc/Paths.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPhysXUsdHumanoidImportTest, "PhysX.Plugin.UsdHumanoidImport", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPhysXUsdHumanoidImportTest::RunTest(const FString& Parameters)
{
	const FString UsdPath = TEXT("C:/Git/ProtoMotions/protomotions/data/assets/usd/smpl_humanoid.usda");
	if (!FPaths::FileExists(UsdPath))
	{
		AddWarning(FString::Printf(TEXT("Skipping USD import smoke test because the sample file is missing: %s"), *UsdPath));
		return true;
	}

	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World)
	{
		AddWarning(TEXT("Skipping USD import smoke test because no editor world is available."));
		return true;
	}

	FPhysXUsdImportOptions Options;
	Options.bCreateSceneActor = true;
	Options.bCreateVisualComponents = false;
	Options.bCreateJointComponents = true;

	const FPhysXUsdImportResult Result = UPhysXUsdImportLibrary::ImportUsdPhysicsScene(World, UsdPath, Options);
	TestTrue(TEXT("USD import succeeded"), Result.bSuccess);
	TestTrue(TEXT("Imported bodies"), Result.NumBodies > 0);
	TestTrue(TEXT("Imported colliders"), Result.NumColliders > 0);
	TestTrue(TEXT("Imported joints"), Result.NumJoints > 0);

	if (Result.RootActor)
	{
		TArray<AActor*> AttachedActors;
		Result.RootActor->GetAttachedActors(AttachedActors);
		for (AActor* Actor : AttachedActors)
		{
			if (Actor)
			{
				Actor->Destroy();
			}
		}
		Result.RootActor->Destroy();
	}

	return true;
}

#endif

#endif
