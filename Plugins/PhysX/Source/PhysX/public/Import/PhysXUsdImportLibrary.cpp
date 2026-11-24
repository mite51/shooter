#include "Import/PhysXUsdImportLibrary.h"

#include "Components/PhysXMaterial.h"
#include "Components/PhysXPvdComponent.h"
#include "Components/PhysXRigidBody.h"
#include "Components/PhysXScene.h"
#include "Components/Collider/BoxPhysXCollider.h"
#include "Components/Collider/CapsulePhysXCollider.h"
#include "Components/Collider/PlanePhysXCollider.h"
#include "Components/Collider/SpherePhysXCollider.h"
#include "Components/Collider/TriangleMeshPhysXCollider.h"
#include "Components/Joint/PhysXD6Joint.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"

#if WITH_EDITOR
#include "MeshDescription.h"
#include "StaticMeshAttributes.h"
#endif

namespace
{
	struct FUsdLimit
	{
		bool bHasLow = false;
		bool bHasHigh = false;
		float Low = 0.0f;
		float High = 0.0f;
	};

	struct FUsdDrive
	{
		float Stiffness = 0.0f;
		float Damping = 0.0f;
		float MaxForce = FLT_MAX;
	};

	struct FUsdPrim
	{
		int32 ParentIndex = INDEX_NONE;
		FString Type;
		FString Name;
		FString Path;
		TSet<FString> Schemas;
		FTransform LocalTransform = FTransform::Identity;
		bool bHasExtent = false;
		FVector ExtentMin = FVector::ZeroVector;
		FVector ExtentMax = FVector::ZeroVector;
		float Radius = 0.0f;
		float Height = 0.0f;
		FString Axis = TEXT("X");
		float Density = 0.0f;
		float StaticFriction = 0.5f;
		float DynamicFriction = 0.5f;
		float Restitution = 0.1f;
		FString Body0Path;
		FString Body1Path;
		FVector LocalPos0 = FVector::ZeroVector;
		FVector LocalPos1 = FVector::ZeroVector;
		FQuat LocalRot0 = FQuat::Identity;
		FQuat LocalRot1 = FQuat::Identity;
		float BreakForce = FLT_MAX;
		float BreakTorque = FLT_MAX;
		FUsdLimit RotLimits[3];
		FUsdLimit TransLimits[3];
		FUsdDrive RotDrives[3];
		TArray<FVector> MeshPoints;
		TArray<int32> FaceVertexCounts;
		TArray<int32> FaceVertexIndices;
	};

	bool ExtractBetween(const FString& Text, TCHAR StartChar, TCHAR EndChar, FString& Out)
	{
		int32 Start = INDEX_NONE;
		int32 End = INDEX_NONE;
		if (Text.FindChar(StartChar, Start) && Text.FindLastChar(EndChar, End) && End > Start)
		{
			Out = Text.Mid(Start + 1, End - Start - 1);
			return true;
		}
		return false;
	}

	TArray<double> ParseNumbers(const FString& Text)
	{
		TArray<double> Values;
		FString Token;

		for (int32 Index = 0; Index < Text.Len(); ++Index)
		{
			const TCHAR Char = Text[Index];
			const bool bNumberChar = FChar::IsDigit(Char) || Char == TEXT('-') || Char == TEXT('+') || Char == TEXT('.') || Char == TEXT('e') || Char == TEXT('E');
			if (bNumberChar)
			{
				Token.AppendChar(Char);
			}
			else if (!Token.IsEmpty())
			{
				Values.Add(FCString::Atod(*Token));
				Token.Reset();
			}
		}

		if (!Token.IsEmpty())
		{
			Values.Add(FCString::Atod(*Token));
		}

		return Values;
	}

	TArray<double> ParseNumbersAfterEquals(const FString& Text)
	{
		FString ValueText = Text;
		int32 EqualsIndex = INDEX_NONE;
		if (Text.FindChar(TEXT('='), EqualsIndex))
		{
			ValueText = Text.Mid(EqualsIndex + 1);
		}
		return ParseNumbers(ValueText);
	}

	float ParseFloatAfterEquals(const FString& Text, float DefaultValue = 0.0f)
	{
		const TArray<double> Values = ParseNumbersAfterEquals(Text);
		return Values.Num() > 0 ? static_cast<float>(Values[0]) : DefaultValue;
	}

	FVector ParseVectorAfterEquals(const FString& Text, float Scale = 1.0f)
	{
		const TArray<double> Values = ParseNumbersAfterEquals(Text);
		return Values.Num() >= 3 ? FVector(Values[0] * Scale, Values[1] * Scale, Values[2] * Scale) : FVector::ZeroVector;
	}

	FQuat ParseUsdQuatAfterEquals(const FString& Text)
	{
		const TArray<double> Values = ParseNumbersAfterEquals(Text);
		return Values.Num() >= 4 ? FQuat(Values[1], Values[2], Values[3], Values[0]).GetNormalized() : FQuat::Identity;
	}

	FTransform ParseMatrixTransform(const FString& Text)
	{
		const TArray<double> Values = ParseNumbersAfterEquals(Text);
		if (Values.Num() < 16)
		{
			return FTransform::Identity;
		}

		const FMatrix Matrix(
			FPlane(Values[0], Values[1], Values[2], Values[3]),
			FPlane(Values[4], Values[5], Values[6], Values[7]),
			FPlane(Values[8], Values[9], Values[10], Values[11]),
			FPlane(Values[12], Values[13], Values[14], Values[15]));
		return FTransform(Matrix);
	}

	TArray<FString> ParseQuotedValues(const FString& Text)
	{
		TArray<FString> Values;
		int32 SearchStart = 0;
		while (SearchStart < Text.Len())
		{
			const FString SearchText = Text.Mid(SearchStart);
			int32 QuoteStart = INDEX_NONE;
			if (!SearchText.FindChar(TEXT('"'), QuoteStart))
			{
				break;
			}
			QuoteStart += SearchStart;
			const FString Remaining = Text.Mid(QuoteStart + 1);
			int32 QuoteEnd = INDEX_NONE;
			if (!Remaining.FindChar(TEXT('"'), QuoteEnd))
			{
				break;
			}
			Values.Add(Remaining.Left(QuoteEnd));
			SearchStart = QuoteStart + QuoteEnd + 2;
		}
		return Values;
	}

	void ParseSchemas(const FString& Line, FUsdPrim& Prim)
	{
		if (!Line.Contains(TEXT("apiSchemas")))
		{
			return;
		}

		for (const FString& Schema : ParseQuotedValues(Line))
		{
			Prim.Schemas.Add(Schema);
		}
	}

	bool ParseDefLine(const FString& Line, FString& OutType, FString& OutName)
	{
		FString AfterDef = Line.RightChop(3).TrimStartAndEnd();
		int32 QuoteStart = INDEX_NONE;
		if (!AfterDef.FindChar(TEXT('"'), QuoteStart))
		{
			return false;
		}

		OutType = AfterDef.Left(QuoteStart).TrimStartAndEnd();
		int32 QuoteEnd = INDEX_NONE;
		const FString AfterQuote = AfterDef.Mid(QuoteStart + 1);
		if (!AfterQuote.FindChar(TEXT('"'), QuoteEnd))
		{
			return false;
		}

		OutName = AfterQuote.Left(QuoteEnd);
		return !OutName.IsEmpty();
	}

	FString MakePrimPath(const TArray<FUsdPrim>& Prims, int32 ParentIndex, const FString& Name)
	{
		const FString ParentPath = Prims.IsValidIndex(ParentIndex) ? Prims[ParentIndex].Path : FString();
		return ParentPath.IsEmpty() ? FString::Printf(TEXT("/%s"), *Name) : FString::Printf(TEXT("%s/%s"), *ParentPath, *Name);
	}

	FString ParseRelationshipPath(const FString& Line)
	{
		FString Path;
		ExtractBetween(Line, TEXT('<'), TEXT('>'), Path);
		return Path;
	}

	void ProcessAttribute(FUsdPrim& Prim, const FString& RawLine)
	{
		const FString Line = RawLine.TrimStartAndEnd();

		if (Line.Contains(TEXT("xformOp:transform")))
		{
			Prim.LocalTransform = ParseMatrixTransform(Line);
		}
		else if (Line.Contains(TEXT("xformOp:translate")))
		{
			Prim.LocalTransform.SetLocation(ParseVectorAfterEquals(Line));
		}
		else if (Line.Contains(TEXT("xformOp:orient")))
		{
			Prim.LocalTransform.SetRotation(ParseUsdQuatAfterEquals(Line));
		}
		else if (Line.Contains(TEXT("xformOp:scale")))
		{
			Prim.LocalTransform.SetScale3D(ParseVectorAfterEquals(Line));
		}
		else if (Line.Contains(TEXT("extent")))
		{
			const TArray<double> Values = ParseNumbersAfterEquals(Line);
			if (Values.Num() >= 6)
			{
				Prim.bHasExtent = true;
				Prim.ExtentMin = FVector(Values[0], Values[1], Values[2]);
				Prim.ExtentMax = FVector(Values[3], Values[4], Values[5]);
			}
		}
		else if (Line.Contains(TEXT(" radius")))
		{
			Prim.Radius = ParseFloatAfterEquals(Line);
		}
		else if (Line.Contains(TEXT(" height")))
		{
			Prim.Height = ParseFloatAfterEquals(Line);
		}
		else if (Line.Contains(TEXT(" axis")))
		{
			const TArray<FString> Tokens = ParseQuotedValues(Line);
			if (Tokens.Num() > 0)
			{
				Prim.Axis = Tokens[0];
			}
		}
		else if (Line.Contains(TEXT("physics:density")))
		{
			Prim.Density = ParseFloatAfterEquals(Line);
		}
		else if (Line.Contains(TEXT("physics:staticFriction")))
		{
			Prim.StaticFriction = ParseFloatAfterEquals(Line, Prim.StaticFriction);
		}
		else if (Line.Contains(TEXT("physics:dynamicFriction")))
		{
			Prim.DynamicFriction = ParseFloatAfterEquals(Line, Prim.DynamicFriction);
		}
		else if (Line.Contains(TEXT("physics:restitution")))
		{
			Prim.Restitution = ParseFloatAfterEquals(Line, Prim.Restitution);
		}
		else if (Line.Contains(TEXT("rel physics:body0")))
		{
			Prim.Body0Path = ParseRelationshipPath(Line);
		}
		else if (Line.Contains(TEXT("rel physics:body1")))
		{
			Prim.Body1Path = ParseRelationshipPath(Line);
		}
		else if (Line.Contains(TEXT("physics:localPos0")))
		{
			Prim.LocalPos0 = ParseVectorAfterEquals(Line);
		}
		else if (Line.Contains(TEXT("physics:localPos1")))
		{
			Prim.LocalPos1 = ParseVectorAfterEquals(Line);
		}
		else if (Line.Contains(TEXT("physics:localRot0")))
		{
			Prim.LocalRot0 = ParseUsdQuatAfterEquals(Line);
		}
		else if (Line.Contains(TEXT("physics:localRot1")))
		{
			Prim.LocalRot1 = ParseUsdQuatAfterEquals(Line);
		}
		else if (Line.Contains(TEXT("physics:breakForce")))
		{
			Prim.BreakForce = ParseFloatAfterEquals(Line, Prim.BreakForce);
		}
		else if (Line.Contains(TEXT("physics:breakTorque")))
		{
			Prim.BreakTorque = ParseFloatAfterEquals(Line, Prim.BreakTorque);
		}
		else if (Line.Contains(TEXT("limit:rotX:physics:low")))
		{
			Prim.RotLimits[0].bHasLow = true;
			Prim.RotLimits[0].Low = ParseFloatAfterEquals(Line);
		}
		else if (Line.Contains(TEXT("limit:rotX:physics:high")))
		{
			Prim.RotLimits[0].bHasHigh = true;
			Prim.RotLimits[0].High = ParseFloatAfterEquals(Line);
		}
		else if (Line.Contains(TEXT("limit:rotY:physics:low")))
		{
			Prim.RotLimits[1].bHasLow = true;
			Prim.RotLimits[1].Low = ParseFloatAfterEquals(Line);
		}
		else if (Line.Contains(TEXT("limit:rotY:physics:high")))
		{
			Prim.RotLimits[1].bHasHigh = true;
			Prim.RotLimits[1].High = ParseFloatAfterEquals(Line);
		}
		else if (Line.Contains(TEXT("limit:rotZ:physics:low")))
		{
			Prim.RotLimits[2].bHasLow = true;
			Prim.RotLimits[2].Low = ParseFloatAfterEquals(Line);
		}
		else if (Line.Contains(TEXT("limit:rotZ:physics:high")))
		{
			Prim.RotLimits[2].bHasHigh = true;
			Prim.RotLimits[2].High = ParseFloatAfterEquals(Line);
		}
		else if (Line.Contains(TEXT("drive:rotX:physics:stiffness")))
		{
			Prim.RotDrives[0].Stiffness = ParseFloatAfterEquals(Line);
		}
		else if (Line.Contains(TEXT("drive:rotX:physics:damping")))
		{
			Prim.RotDrives[0].Damping = ParseFloatAfterEquals(Line);
		}
		else if (Line.Contains(TEXT("drive:rotX:physics:maxForce")))
		{
			Prim.RotDrives[0].MaxForce = ParseFloatAfterEquals(Line, FLT_MAX);
		}
		else if (Line.Contains(TEXT("drive:rotY:physics:stiffness")))
		{
			Prim.RotDrives[1].Stiffness = ParseFloatAfterEquals(Line);
		}
		else if (Line.Contains(TEXT("drive:rotY:physics:damping")))
		{
			Prim.RotDrives[1].Damping = ParseFloatAfterEquals(Line);
		}
		else if (Line.Contains(TEXT("drive:rotY:physics:maxForce")))
		{
			Prim.RotDrives[1].MaxForce = ParseFloatAfterEquals(Line, FLT_MAX);
		}
		else if (Line.Contains(TEXT("drive:rotZ:physics:stiffness")))
		{
			Prim.RotDrives[2].Stiffness = ParseFloatAfterEquals(Line);
		}
		else if (Line.Contains(TEXT("drive:rotZ:physics:damping")))
		{
			Prim.RotDrives[2].Damping = ParseFloatAfterEquals(Line);
		}
		else if (Line.Contains(TEXT("drive:rotZ:physics:maxForce")))
		{
			Prim.RotDrives[2].MaxForce = ParseFloatAfterEquals(Line, FLT_MAX);
		}
		else if (Line.Contains(TEXT("point3f[] points")))
		{
			const TArray<double> Values = ParseNumbersAfterEquals(Line);
			for (int32 Index = 0; Index + 2 < Values.Num(); Index += 3)
			{
				Prim.MeshPoints.Add(FVector(Values[Index], Values[Index + 1], Values[Index + 2]));
			}
		}
		else if (Line.Contains(TEXT("int[] faceVertexCounts")))
		{
			for (double Value : ParseNumbersAfterEquals(Line))
			{
				Prim.FaceVertexCounts.Add(static_cast<int32>(Value));
			}
		}
		else if (Line.Contains(TEXT("int[] faceVertexIndices")))
		{
			for (double Value : ParseNumbersAfterEquals(Line))
			{
				Prim.FaceVertexIndices.Add(static_cast<int32>(Value));
			}
		}
	}

	TArray<FUsdPrim> ParseUsdAscii(const FString& Text)
	{
		TArray<FUsdPrim> Prims;
		TArray<FString> Lines;
		Text.ParseIntoArrayLines(Lines);

		TArray<int32> Stack;
		int32 PendingPrimIndex = INDEX_NONE;

		for (int32 LineIndex = 0; LineIndex < Lines.Num(); ++LineIndex)
		{
			FString Line = Lines[LineIndex].TrimStartAndEnd();
			if (Line.IsEmpty() || Line.StartsWith(TEXT("#")))
			{
				continue;
			}

			if ((Line.Contains(TEXT("points")) || Line.Contains(TEXT("faceVertexCounts")) || Line.Contains(TEXT("faceVertexIndices"))) && Line.Contains(TEXT("[")) && !Line.Contains(TEXT("]")))
			{
				while (LineIndex + 1 < Lines.Num() && !Line.Contains(TEXT("]")))
				{
					Line += TEXT(" ");
					Line += Lines[++LineIndex].TrimStartAndEnd();
				}
			}

			if (Line.StartsWith(TEXT("def ")))
			{
				FString Type;
				FString Name;
				if (ParseDefLine(Line, Type, Name))
				{
					FUsdPrim& Prim = Prims.AddDefaulted_GetRef();
					Prim.ParentIndex = Stack.Num() > 0 ? Stack.Last() : INDEX_NONE;
					Prim.Type = Type;
					Prim.Name = Name;
					Prim.Path = MakePrimPath(Prims, Prim.ParentIndex, Name);
					ParseSchemas(Line, Prim);
					PendingPrimIndex = Prims.Num() - 1;
				}
			}
			else if (PendingPrimIndex != INDEX_NONE)
			{
				ParseSchemas(Line, Prims[PendingPrimIndex]);
			}
			else if (Stack.Num() > 0)
			{
				ProcessAttribute(Prims[Stack.Last()], Line);
			}

			if (Line.Contains(TEXT("{")) && PendingPrimIndex != INDEX_NONE)
			{
				Stack.Add(PendingPrimIndex);
				PendingPrimIndex = INDEX_NONE;
			}

			if (Line.StartsWith(TEXT("}")))
			{
				int32 CloseCount = 0;
				for (TCHAR Char : Line)
				{
					if (Char == TEXT('}'))
					{
						++CloseCount;
					}
				}
				while (CloseCount-- > 0 && Stack.Num() > 0)
				{
					Stack.Pop();
				}
			}
		}

		return Prims;
	}

	FTransform GetWorldTransformRaw(const TArray<FUsdPrim>& Prims, int32 PrimIndex)
	{
		if (!Prims.IsValidIndex(PrimIndex))
		{
			return FTransform::Identity;
		}

		const FTransform ParentTransform = GetWorldTransformRaw(Prims, Prims[PrimIndex].ParentIndex);
		return Prims[PrimIndex].LocalTransform * ParentTransform;
	}

	FTransform ConvertUsdTransform(const FTransform& Transform, float Units)
	{
		FTransform Converted = Transform;
		Converted.SetLocation(Transform.GetLocation() * Units);
		return Converted;
	}

	FName MakeObjectName(const FString& Prefix, const FString& Name)
	{
		FString Sanitized = Prefix + TEXT("_") + Name;
		Sanitized.ReplaceInline(TEXT("/"), TEXT("_"));
		Sanitized.ReplaceInline(TEXT(" "), TEXT("_"));
		return FName(*Sanitized);
	}

	USceneComponent* EnsureRootComponent(AActor* Actor)
	{
		if (USceneComponent* ExistingRoot = Actor->GetRootComponent())
		{
			return ExistingRoot;
		}

		USceneComponent* Root = NewObject<USceneComponent>(Actor, TEXT("Root"));
		Actor->SetRootComponent(Root);
		Actor->AddInstanceComponent(Root);
		Root->RegisterComponent();
		return Root;
	}

	template <typename TComponent>
	TComponent* AddActorComponent(AActor* Actor, const FName Name)
	{
		TComponent* Component = NewObject<TComponent>(Actor, Name);
		Actor->AddInstanceComponent(Component);
		Component->RegisterComponent();
		return Component;
	}

	template <typename TComponent>
	TComponent* AddSceneComponent(AActor* Actor, const FName Name)
	{
		TComponent* Component = NewObject<TComponent>(Actor, Name);
		Component->SetupAttachment(EnsureRootComponent(Actor));
		Actor->AddInstanceComponent(Component);
		Component->RegisterComponent();
		return Component;
	}

	UStaticMesh* LoadBasicMesh(const TCHAR* Path)
	{
		return LoadObject<UStaticMesh>(nullptr, Path);
	}

#if WITH_EDITOR
	UStaticMesh* CreateTransientStaticMesh(const FUsdPrim& Prim, float Units)
	{
		if (Prim.MeshPoints.Num() == 0 || Prim.FaceVertexCounts.Num() == 0 || Prim.FaceVertexIndices.Num() == 0)
		{
			return nullptr;
		}

		FMeshDescription MeshDescription;
		FStaticMeshAttributes Attributes(MeshDescription);
		Attributes.Register();

		TVertexAttributesRef<FVector3f> Positions = Attributes.GetVertexPositions();
		TArray<FVertexID> VertexIds;
		VertexIds.Reserve(Prim.MeshPoints.Num());
		for (const FVector& Point : Prim.MeshPoints)
		{
			const FVertexID VertexId = MeshDescription.CreateVertex();
			Positions[VertexId] = FVector3f(Point * Units);
			VertexIds.Add(VertexId);
		}

		const FPolygonGroupID PolygonGroupId = MeshDescription.CreatePolygonGroup();
		int32 CornerIndex = 0;
		for (int32 FaceVertexCount : Prim.FaceVertexCounts)
		{
			TArray<FVertexInstanceID> VertexInstanceIds;
			for (int32 FaceCorner = 0; FaceCorner < FaceVertexCount && Prim.FaceVertexIndices.IsValidIndex(CornerIndex); ++FaceCorner, ++CornerIndex)
			{
				const int32 VertexIndex = Prim.FaceVertexIndices[CornerIndex];
				if (VertexIds.IsValidIndex(VertexIndex))
				{
					VertexInstanceIds.Add(MeshDescription.CreateVertexInstance(VertexIds[VertexIndex]));
				}
			}

			if (VertexInstanceIds.Num() >= 3)
			{
				MeshDescription.CreatePolygon(PolygonGroupId, VertexInstanceIds);
			}
		}

		UStaticMesh* Mesh = NewObject<UStaticMesh>(GetTransientPackageAsObject(), MakeObjectName(TEXT("USDMesh"), Prim.Name), RF_Transient);
		TArray<const FMeshDescription*> MeshDescriptions;
		MeshDescriptions.Add(&MeshDescription);
		Mesh->BuildFromMeshDescriptions(MeshDescriptions);
		return Mesh;
	}
#endif

	void AddVisualComponent(AActor* Actor, const FUsdPrim& Prim, const FTransform& RelativeRaw, float Units)
	{
		UStaticMesh* Mesh = nullptr;
		FVector RelativeScale = FVector::OneVector;

		if (Prim.Type == TEXT("Cube"))
		{
			Mesh = LoadBasicMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
			const FVector Size = Prim.bHasExtent ? (Prim.ExtentMax - Prim.ExtentMin) * Units : FVector(100.0f);
			RelativeScale = Size / 100.0f;
		}
		else if (Prim.Type == TEXT("Sphere"))
		{
			Mesh = LoadBasicMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
			const float Diameter = (Prim.Radius > 0.0f ? Prim.Radius * 2.0f * Units : 100.0f);
			RelativeScale = FVector(Diameter / 100.0f);
		}
		else if (Prim.Type == TEXT("Capsule"))
		{
			Mesh = LoadBasicMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
			const float Radius = Prim.Radius > 0.0f ? Prim.Radius * Units : 50.0f;
			const float Height = Prim.Height > 0.0f ? Prim.Height * Units : 100.0f;
			RelativeScale = FVector(Radius * 2.0f / 100.0f, Radius * 2.0f / 100.0f, Height / 100.0f);
		}
		else if (Prim.Type == TEXT("Mesh"))
		{
#if WITH_EDITOR
			Mesh = CreateTransientStaticMesh(Prim, Units);
#endif
		}

		if (!Mesh)
		{
			return;
		}

		UStaticMeshComponent* Visual = AddSceneComponent<UStaticMeshComponent>(Actor, MakeObjectName(TEXT("USDVisual"), Prim.Name));
		FTransform RelativeTransform = ConvertUsdTransform(RelativeRaw, Units);
		RelativeTransform.SetScale3D(RelativeScale);
		Visual->SetRelativeTransform(RelativeTransform);
		Visual->SetStaticMesh(Mesh);
	}

	FQuat MakeCapsuleAxisAdjustment(const FString& Axis)
	{
		if (Axis.Equals(TEXT("Y"), ESearchCase::IgnoreCase))
		{
			return FQuat::FindBetweenNormals(FVector::XAxisVector, FVector::YAxisVector);
		}
		if (Axis.Equals(TEXT("Z"), ESearchCase::IgnoreCase))
		{
			return FQuat::FindBetweenNormals(FVector::XAxisVector, FVector::ZAxisVector);
		}
		return FQuat::Identity;
	}

	void AddColliderComponent(AActor* Actor, const FUsdPrim& Prim, const FTransform& RelativeRaw, float Units, FPhysXUsdImportResult& Result)
	{
		const FVector LocalPosition = RelativeRaw.GetLocation() * Units;
		const FQuat LocalRotation = RelativeRaw.GetRotation();

		if (Prim.Type == TEXT("Cube"))
		{
			UBoxPhysXCollider* Collider = AddActorComponent<UBoxPhysXCollider>(Actor, MakeObjectName(TEXT("USDCollider"), Prim.Name));
			Collider->LocalPosition = LocalPosition;
			Collider->LocalRotation = LocalRotation;
			Collider->HalfExtents = Prim.bHasExtent ? (Prim.ExtentMax - Prim.ExtentMin) * 0.5f * Units : FVector(50.0f);
			++Result.NumColliders;
		}
		else if (Prim.Type == TEXT("Capsule"))
		{
			UCapsulePhysXCollider* Collider = AddActorComponent<UCapsulePhysXCollider>(Actor, MakeObjectName(TEXT("USDCollider"), Prim.Name));
			Collider->LocalPosition = LocalPosition;
			Collider->LocalRotation = MakeCapsuleAxisAdjustment(Prim.Axis) * LocalRotation;
			Collider->Radius = Prim.Radius > 0.0f ? Prim.Radius * Units : 50.0f;
			Collider->HalfHeight = Prim.Height > 0.0f ? Prim.Height * 0.5f * Units : 50.0f;
			++Result.NumColliders;
		}
		else if (Prim.Type == TEXT("Sphere"))
		{
			USpherePhysXCollider* Collider = AddActorComponent<USpherePhysXCollider>(Actor, MakeObjectName(TEXT("USDCollider"), Prim.Name));
			Collider->LocalPosition = LocalPosition;
			Collider->LocalRotation = LocalRotation;
			Collider->Radius = Prim.Radius > 0.0f ? Prim.Radius * Units : 50.0f;
			++Result.NumColliders;
		}
		else if (Prim.Type == TEXT("Plane"))
		{
			UPlanePhysXCollider* Collider = AddActorComponent<UPlanePhysXCollider>(Actor, MakeObjectName(TEXT("USDCollider"), Prim.Name));
			Collider->LocalPosition = LocalPosition;
			Collider->LocalRotation = LocalRotation;
			++Result.NumColliders;
		}
	}

	EPxD6Motion MotionFromLimit(const FUsdLimit& Limit)
	{
		return (Limit.bHasLow || Limit.bHasHigh) ? EPxD6Motion::Limited : EPxD6Motion::Free;
	}

	FPxD6JointDrive MakeDrive(const FUsdDrive& Drive)
	{
		FPxD6JointDrive Result;
		Result.Stiffness = Drive.Stiffness;
		Result.Damping = Drive.Damping;
		Result.ForceLimit = Drive.MaxForce;
		Result.bAcceleration = false;
		return Result;
	}

	void ConfigureJoint(UPhysXD6Joint* Joint, const FUsdPrim& Prim, const TMap<FString, AActor*>& BodyActors, float Units)
	{
		Joint->Actor0 = BodyActors.FindRef(Prim.Body0Path);
		Joint->Actor1 = BodyActors.FindRef(Prim.Body1Path);
		Joint->LocalPose0 = FTransform(Prim.LocalRot0, Prim.LocalPos0 * Units);
		Joint->LocalPose1 = FTransform(Prim.LocalRot1, Prim.LocalPos1 * Units);
		Joint->BreakForce = Prim.BreakForce;
		Joint->BreakTorque = Prim.BreakTorque;

		Joint->MotionX = EPxD6Motion::Locked;
		Joint->MotionY = EPxD6Motion::Locked;
		Joint->MotionZ = EPxD6Motion::Locked;
		Joint->MotionTwist = MotionFromLimit(Prim.RotLimits[0]);
		Joint->MotionSwing1 = MotionFromLimit(Prim.RotLimits[1]);
		Joint->MotionSwing2 = MotionFromLimit(Prim.RotLimits[2]);

		if (Prim.RotLimits[0].bHasLow || Prim.RotLimits[0].bHasHigh)
		{
			Joint->TwistLimit = FVector2D(
				FMath::DegreesToRadians(Prim.RotLimits[0].bHasLow ? Prim.RotLimits[0].Low : -180.0f),
				FMath::DegreesToRadians(Prim.RotLimits[0].bHasHigh ? Prim.RotLimits[0].High : 180.0f));
		}
		if (Prim.RotLimits[1].bHasLow || Prim.RotLimits[1].bHasHigh)
		{
			Joint->SwingLimitY = FMath::DegreesToRadians(FMath::Max(FMath::Abs(Prim.RotLimits[1].Low), FMath::Abs(Prim.RotLimits[1].High)));
		}
		if (Prim.RotLimits[2].bHasLow || Prim.RotLimits[2].bHasHigh)
		{
			Joint->SwingLimitZ = FMath::DegreesToRadians(FMath::Max(FMath::Abs(Prim.RotLimits[2].Low), FMath::Abs(Prim.RotLimits[2].High)));
		}

		Joint->DriveTwist = MakeDrive(Prim.RotDrives[0]);
		Joint->DriveSwing = MakeDrive(Prim.RotDrives[1]);
		Joint->DriveSlerp = MakeDrive(Prim.RotDrives[2]);
	}
}

FPhysXUsdImportOptions::FPhysXUsdImportOptions()
{
	MetersToUnrealUnits = 100.0f;
	bCreateSceneActor = true;
	bCreateVisualComponents = true;
	bCreateJointComponents = true;
	bAutoInitializeScene = false;
}

FPhysXUsdImportResult::FPhysXUsdImportResult()
{
	bSuccess = false;
	RootActor = nullptr;
	NumBodies = 0;
	NumColliders = 0;
	NumVisuals = 0;
	NumJoints = 0;
}

FPhysXUsdImportResult UPhysXUsdImportLibrary::ImportUsdPhysicsScene(UObject* WorldContextObject, const FString& UsdFilePath, const FPhysXUsdImportOptions& Options)
{
	FPhysXUsdImportResult Result;

	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	if (!World)
	{
		Result.Message = TEXT("No valid world context for USD import.");
		return Result;
	}

	FString UsdText;
	if (!FFileHelper::LoadFileToString(UsdText, *UsdFilePath))
	{
		Result.Message = FString::Printf(TEXT("Could not read USD file: %s"), *UsdFilePath);
		return Result;
	}

	const TArray<FUsdPrim> Prims = ParseUsdAscii(UsdText);
	if (Prims.Num() == 0)
	{
		Result.Message = FString::Printf(TEXT("No USD prims were parsed from %s"), *UsdFilePath);
		return Result;
	}

	const FString BaseName = FPaths::GetBaseFilename(UsdFilePath);
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Name = MakeObjectName(TEXT("PhysXUSD"), BaseName);
	AActor* RootActor = World->SpawnActor<AActor>(SpawnParameters);
	EnsureRootComponent(RootActor);
	Result.RootActor = RootActor;

	UPhysXScene* SceneComponent = nullptr;
	if (Options.bCreateSceneActor)
	{
		SceneComponent = AddActorComponent<UPhysXScene>(RootActor, TEXT("PhysXScene"));
		UPhysXPvdComponent* PvdComponent = AddActorComponent<UPhysXPvdComponent>(RootActor, TEXT("PhysXPvd"));
		PvdComponent->bConnectToPvd = false;
	}

	TMap<FString, AActor*> BodyActors;
	TMap<FString, int32> PrimIndexByPath;
	for (int32 PrimIndex = 0; PrimIndex < Prims.Num(); ++PrimIndex)
	{
		PrimIndexByPath.Add(Prims[PrimIndex].Path, PrimIndex);
	}

	for (int32 PrimIndex = 0; PrimIndex < Prims.Num(); ++PrimIndex)
	{
		const FUsdPrim& Prim = Prims[PrimIndex];
		if (!Prim.Schemas.Contains(TEXT("PhysicsRigidBodyAPI")) && !Prim.Schemas.Contains(TEXT("PhysxRigidBodyAPI")))
		{
			continue;
		}

		FActorSpawnParameters BodySpawnParameters;
		BodySpawnParameters.Name = MakeObjectName(TEXT("PhysXBody"), Prim.Name);
		AActor* BodyActor = World->SpawnActor<AActor>(BodySpawnParameters);
		EnsureRootComponent(BodyActor);
		BodyActor->AttachToActor(RootActor, FAttachmentTransformRules::KeepWorldTransform);
		BodyActor->SetActorTransform(ConvertUsdTransform(GetWorldTransformRaw(Prims, PrimIndex), Options.MetersToUnrealUnits));

		UPhysXRigidBody* RigidBody = AddActorComponent<UPhysXRigidBody>(BodyActor, TEXT("PhysXRigidBody"));
		RigidBody->BodyType = EPhysXRigidBodyType::Dynamic;
		RigidBody->Density = Prim.Density;
		RigidBody->bComputeMassFromShapes = Prim.Density > 0.0f;

		if (Prim.Schemas.Contains(TEXT("PhysicsMaterialAPI")))
		{
			UPhysXMaterial* Material = AddActorComponent<UPhysXMaterial>(BodyActor, TEXT("PhysXMaterial"));
			Material->StaticFriction = Prim.StaticFriction;
			Material->DynamicFriction = Prim.DynamicFriction;
			Material->Restitution = Prim.Restitution;
			Material->Density = Prim.Density;
		}

		BodyActors.Add(Prim.Path, BodyActor);
		++Result.NumBodies;

		const FTransform BodyWorldRaw = GetWorldTransformRaw(Prims, PrimIndex);
		for (int32 ChildPrimIndex = 0; ChildPrimIndex < Prims.Num(); ++ChildPrimIndex)
		{
			const FUsdPrim& ChildPrim = Prims[ChildPrimIndex];
			const bool bIsCollision = ChildPrim.Path.StartsWith(Prim.Path + TEXT("/collisions/")) && ChildPrim.Schemas.Contains(TEXT("PhysicsCollisionAPI"));
			const bool bIsVisual = Options.bCreateVisualComponents && ChildPrim.Path.StartsWith(Prim.Path + TEXT("/visuals/"));
			if (!bIsCollision && !bIsVisual)
			{
				continue;
			}

			const FTransform ChildWorldRaw = GetWorldTransformRaw(Prims, ChildPrimIndex);
			const FTransform RelativeRaw = ChildWorldRaw.GetRelativeTransform(BodyWorldRaw);

			if (bIsCollision)
			{
				AddColliderComponent(BodyActor, ChildPrim, RelativeRaw, Options.MetersToUnrealUnits, Result);
			}
			else if (bIsVisual)
			{
				AddVisualComponent(BodyActor, ChildPrim, RelativeRaw, Options.MetersToUnrealUnits);
				++Result.NumVisuals;
			}
		}
	}

	if (Options.bCreateJointComponents)
	{
		for (const FUsdPrim& Prim : Prims)
		{
			if (Prim.Type != TEXT("PhysicsJoint"))
			{
				continue;
			}

			AActor* JointOwner = BodyActors.FindRef(Prim.Body1Path);
			if (!JointOwner)
			{
				JointOwner = RootActor;
			}

			UPhysXD6Joint* Joint = AddActorComponent<UPhysXD6Joint>(JointOwner, MakeObjectName(TEXT("PhysXJoint"), Prim.Name));
			ConfigureJoint(Joint, Prim, BodyActors, Options.MetersToUnrealUnits);
			++Result.NumJoints;
		}
	}

	if (SceneComponent && Options.bAutoInitializeScene)
	{
		SceneComponent->InitializePhysXSimulation();
	}

	Result.bSuccess = true;
	Result.Message = FString::Printf(TEXT("Imported %d bodies, %d colliders, %d visuals, and %d joints from %s"), Result.NumBodies, Result.NumColliders, Result.NumVisuals, Result.NumJoints, *UsdFilePath);
	return Result;
}
