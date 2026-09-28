// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/BoxComponent.h"
#include "Components/DecalComponent.h"
#include "Engine/Engine.h"
#include "Engine/Scene.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "FeelBlueprintLibrary.h"
#include "FeelFrameOutput.h"
#include "FeelPlayCapture.h"
#include "FeelRecipe.h"
#include "FeelSettings.h"
#include "FeelSubsystem.h"
#include "FeelTags.h"
#include "GameFramework/Actor.h"
#include "MaterialDomain.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/App.h"
#include "Misc/ScopeExit.h"
#include "Components/StaticMeshComponent.h"
#include "Steps/FeelStep_ScalePunch.h"
#include "Steps/FeelStep_ScreenFlash.h"
#include "Steps/FeelStep_Spawn.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UObjectIterator.h"

#define FEEL_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

namespace FeelRetestFixesTests
{
	UWorld* CreateTestWorld()
	{
		UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
		FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
		Context.SetCurrentWorld(World);
		World->InitializeActorsForPlay(FURL());
		World->BeginPlay();
		return World;
	}

	void DestroyTestWorld(UWorld* World)
	{
		World->RemoveFromRoot();
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelPostProcessWeightParameterTest, "FeelKit.Output.PostProcessMaterialWeightParameter", FEEL_TEST_FLAGS)
bool FFeelPostProcessWeightParameterTest::RunTest(const FString& Parameters)
{
	UMaterial* Material = UMaterial::GetDefaultMaterial(MD_Surface);
	FFeelOutputAccumulator Accumulator;
	Accumulator.AddPostProcessMaterial(Material, 0.4f, TEXT("Weight"));

	// Without instances (old path): the material itself with the weight.
	int32 Blends = 0;
	Accumulator.Output.ForEachPostProcessBlend([&](FPostProcessSettings& Settings, float Weight)
	{
		++Blends;
		TestEqual(TEXT("Without instances the weight is passed as the blend weight"), Weight, 0.4f, 0.001f);
		TestTrue(TEXT("Without instances the material itself is blended"), Settings.WeightedBlendables.Array.Num() == 1 && Settings.WeightedBlendables.Array[0].Object == Material);
	});
	TestEqual(TEXT("One material blend"), Blends, 1);

	// With instances: a material instance carries the weight in its parameter, blended at full weight.
	FFeelPostProcessMaterialInstances Instances;
	UMaterialInstanceDynamic* FirstInstance = nullptr;
	Accumulator.Output.ForEachPostProcessBlend([&](FPostProcessSettings& Settings, float Weight)
	{
		TestEqual(TEXT("The instance is blended at full weight"), Weight, 1.0f, 0.001f);
		FirstInstance = Settings.WeightedBlendables.Array.Num() == 1 ? Cast<UMaterialInstanceDynamic>(Settings.WeightedBlendables.Array[0].Object) : nullptr;
	}, &Instances);
	if (TestNotNull(TEXT("A material instance is blended"), FirstInstance))
	{
		TestTrue(TEXT("The instance is of the step's material"), FirstInstance->Parent == Material);
		float Value = -1.0f;
		TestTrue(TEXT("The instance has the weight parameter set"), FirstInstance->GetScalarParameterValue(FMaterialParameterInfo(TEXT("Weight")), Value, true));
		TestEqual(TEXT("The parameter holds the weight"), Value, 0.4f, 0.001f);
	}

	// The same instance is reused frame to frame with the new weight.
	FFeelOutputAccumulator Next;
	Next.AddPostProcessMaterial(Material, 0.9f, TEXT("Weight"));
	UMaterialInstanceDynamic* SecondInstance = nullptr;
	Next.Output.ForEachPostProcessBlend([&](FPostProcessSettings& Settings, float)
	{
		SecondInstance = Cast<UMaterialInstanceDynamic>(Settings.WeightedBlendables.Array[0].Object);
	}, &Instances);
	TestTrue(TEXT("The instance is reused"), SecondInstance == FirstInstance);
	float Value = -1.0f;
	if (SecondInstance && SecondInstance->GetScalarParameterValue(FMaterialParameterInfo(TEXT("Weight")), Value, true))
	{
		TestEqual(TEXT("The reused instance has the new weight"), Value, 0.9f, 0.001f);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelSpawnDecalSurfaceTest, "FeelKit.Steps.SpawnDecalFindsSurface", FEEL_TEST_FLAGS)
bool FFeelSpawnDecalSurfaceTest::RunTest(const FString& Parameters)
{
	using namespace FeelRetestFixesTests;
	UWorld* World = CreateTestWorld();
	ON_SCOPE_EXIT
	{
		DestroyTestWorld(World);
	};

	// A floor whose top is at Z = 0.
	AActor* Floor = World->SpawnActor<AActor>();
	UBoxComponent* Box = NewObject<UBoxComponent>(Floor, TEXT("Floor"));
	Box->SetBoxExtent(FVector(500.0, 500.0, 50.0));
	Box->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	Floor->SetRootComponent(Box);
	Box->RegisterComponent();
	Box->SetWorldLocation(FVector(0.0, 0.0, -50.0));

	TStrongObjectPtr<UFeelRecipe> Recipe(NewObject<UFeelRecipe>(GetTransientPackage()));
	UFeelStep_SpawnDecal* Decal = NewObject<UFeelStep_SpawnDecal>(Recipe.Get());
	Decal->DecalMaterial = UMaterial::GetDefaultMaterial(MD_DeferredDecal);
	Decal->bRandomRotation = false;
	{
		FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
		Track.Step = Decal;
		Track.Channel = FeelTags::Spawn;
		Track.Duration = 0.1f;
	}

	UFeelSubsystem* Subsystem = World->GetSubsystem<UFeelSubsystem>();
	TSet<UDecalComponent*> Before;
	for (TObjectIterator<UDecalComponent> It; It; ++It)
	{
		Before.Add(*It);
	}

	// Played 90 cm above the floor, like a character's location.
	Subsystem->PlayFeel(Recipe.Get(), FFeelTarget::AtLocation(FVector(10.0, 20.0, 90.0)));
	FApp::SetDeltaTime(0.02f);
	Subsystem->Tick(0.02f);

	UDecalComponent* Spawned = nullptr;
	for (TObjectIterator<UDecalComponent> It; It; ++It)
	{
		if (!Before.Contains(*It) && It->GetWorld() == World)
		{
			Spawned = *It;
		}
	}
	if (TestNotNull(TEXT("A decal was spawned"), Spawned))
	{
		const FVector Location = Spawned->GetComponentLocation();
		TestEqual(TEXT("The decal sits on the floor surface below the play location"), Location.Z, 0.0, 1.0);
		TestEqual(TEXT("The decal keeps the play location's X"), Location.X, 10.0, 1.0);
		TestTrue(TEXT("The decal projects down into the floor"), Spawned->GetForwardVector().Equals(FVector::DownVector, 0.01));
		TestFalse(TEXT("Fading never destroys the decal's owner"), Spawned->bDestroyOwnerAfterFade);
	}

	// A character-sized target: its location is 96 cm above the floor, more than a 90 cm search. The search starts at
	// the target's edge, so the floor is still found.
	{
		AActor* Character = World->SpawnActor<AActor>();
		UBoxComponent* Capsule = NewObject<UBoxComponent>(Character, TEXT("Capsule"));
		Capsule->SetBoxExtent(FVector(35.0, 35.0, 96.0));
		Capsule->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
		Character->SetRootComponent(Capsule);
		Capsule->RegisterComponent();
		Capsule->SetWorldLocation(FVector(100.0, 0.0, 96.0));
		Decal->SurfaceSearchDistance = 90.0f;
		for (TObjectIterator<UDecalComponent> It; It; ++It)
		{
			Before.Add(*It);
		}
		Subsystem->PlayFeel(Recipe.Get(), FFeelTarget::FromActor(Character));
		Subsystem->Tick(0.02f);
		UDecalComponent* UnderTarget = nullptr;
		for (TObjectIterator<UDecalComponent> It; It; ++It)
		{
			if (!Before.Contains(*It) && It->GetWorld() == World)
			{
				UnderTarget = *It;
			}
		}
		if (TestNotNull(TEXT("A decal was spawned for the character-sized target"), UnderTarget))
		{
			TestEqual(TEXT("A 90 cm search from a character-sized target reaches the floor"), UnderTarget->GetComponentLocation().Z, 0.0, 1.0);
		}
	}

	// Without a surface in range it spawns at the play location.
	Decal->SurfaceSearchDistance = 20.0f;
	for (TObjectIterator<UDecalComponent> It; It; ++It)
	{
		Before.Add(*It);
	}
	Subsystem->PlayFeel(Recipe.Get(), FFeelTarget::AtLocation(FVector(0.0, 0.0, 300.0)));
	Subsystem->Tick(0.02f);
	UDecalComponent* Unsupported = nullptr;
	for (TObjectIterator<UDecalComponent> It; It; ++It)
	{
		if (!Before.Contains(*It) && It->GetWorld() == World)
		{
			Unsupported = *It;
		}
	}
	if (TestNotNull(TEXT("A decal was spawned without a surface"), Unsupported))
	{
		TestEqual(TEXT("With no surface in range the decal stays at the play location"), Unsupported->GetComponentLocation().Z, 300.0, 1.0);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelCaptureOnWorldEndTest, "FeelKit.Proof.CaptureWhenWorldEndsMidPlay", FEEL_TEST_FLAGS)
bool FFeelCaptureOnWorldEndTest::RunTest(const FString& Parameters)
{
	using namespace FeelRetestFixesTests;
	FFeelPlayCaptureStore::Get().Clear();

	TStrongObjectPtr<UFeelRecipe> Recipe(NewObject<UFeelRecipe>(GetTransientPackage()));
	{
		FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
		Track.Step = NewObject<UFeelStep_ScreenFlash>(Recipe.Get());
		Track.Channel = FeelTags::Screen_Flash;
		Track.Duration = 10.0f;
	}

	UWorld* World = CreateTestWorld();
	UFeelSubsystem* Subsystem = World->GetSubsystem<UFeelSubsystem>();
	Subsystem->PlayFeel(Recipe.Get(), FFeelTarget(), 0.6f);
	FApp::SetDeltaTime(0.1f);
	Subsystem->Tick(0.1f);
	Subsystem->Tick(0.1f);

	// Like stopping Play In Editor in the middle of the play.
	DestroyTestWorld(World);

	const TArray<FFeelPlayCapture>& Captures = FFeelPlayCaptureStore::Get().GetCaptures();
#if UE_BUILD_SHIPPING
	TestEqual(TEXT("Shipping records nothing"), Captures.Num(), 0);
#else
	if (TestEqual(TEXT("The play cut short by the world ending is recorded"), Captures.Num(), 1))
	{
		TestTrue(TEXT("It is marked as stopped early"), Captures[0].bInterrupted);
		TestEqual(TEXT("It keeps the intensity"), Captures[0].Intensity, 0.6f);
		TestTrue(TEXT("It keeps the comfort of its last frame"), Captures[0].bHasComfort);
		TestTrue(TEXT("It points at the recipe"), Captures[0].Recipe.Get() == Recipe.Get());
	}
#endif
	FFeelPlayCaptureStore::Get().Clear();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelAccumulatorOptionsTest, "FeelKit.Blueprint.AccumulatorNameOptions", FEEL_TEST_FLAGS)
bool FFeelAccumulatorOptionsTest::RunTest(const FString& Parameters)
{
	UFeelSettings* Settings = GetMutableDefault<UFeelSettings>();
	const TArray<FFeelAccumulatorDefinition> Saved = Settings->Accumulators;
	ON_SCOPE_EXIT
	{
		Settings->Accumulators = Saved;
	};

	Settings->Accumulators.Reset();
	Settings->Accumulators.AddDefaulted_GetRef().Name = TEXT("TestCombo");
	Settings->Accumulators.AddDefaulted_GetRef().Name = TEXT("TestHeat");
	Settings->Accumulators.AddDefaulted();

	const TArray<FName> Options = UFeelBlueprintLibrary::GetFeelAccumulatorOptions();
	TestEqual(TEXT("Named accumulators are offered, unnamed ones are not"), Options.Num(), 2);
	TestTrue(TEXT("Options list the settings' accumulators"), Options.Contains(FName(TEXT("TestCombo"))) && Options.Contains(FName(TEXT("TestHeat"))));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelActorMotionKeepsCollisionTest, "FeelKit.Runtime.ActorMotionKeepsCollisionRoot", FEEL_TEST_FLAGS)
bool FFeelActorMotionKeepsCollisionTest::RunTest(const FString& Parameters)
{
	// A character-like actor: a collision root with a visible child. Scaling the root would resize its collision and move the
	// actor (and replicate that movement to other machines), so actor motion goes to the visible child.
	UWorld* World = FeelRetestFixesTests::CreateTestWorld();
	const double PreviousDeltaTime = FApp::GetDeltaTime();
	ON_SCOPE_EXIT
	{
		FApp::SetDeltaTime(PreviousDeltaTime);
		FeelRetestFixesTests::DestroyTestWorld(World);
	};
	UFeelSubsystem* Subsystem = World->GetSubsystem<UFeelSubsystem>();

	AActor* Actor = World->SpawnActor<AActor>();
	UBoxComponent* Collision = NewObject<UBoxComponent>(Actor, TEXT("Collision"));
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Actor->SetRootComponent(Collision);
	Collision->RegisterComponent();
	UStaticMeshComponent* Mesh = NewObject<UStaticMeshComponent>(Actor, TEXT("Mesh"));
	Mesh->SetupAttachment(Collision);
	Mesh->RegisterComponent();
	USceneComponent* Pivot = NewObject<USceneComponent>(Actor, TEXT("Pivot"));
	Pivot->SetupAttachment(Collision);
	Pivot->RegisterComponent();

	UFeelRecipe* Recipe = NewObject<UFeelRecipe>(GetTransientPackage());
	UFeelStep_ScalePunch* Punch = NewObject<UFeelStep_ScalePunch>(Recipe);
	Punch->Amount = FVector(0.5);
	Punch->Bounces = 0;
	FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
	Track.Step = Punch;
	Track.Duration = 0.4f;
	Track.IntensityCurve.GetRichCurve()->Reset();

	auto Step = [Subsystem](float Seconds)
	{
		FApp::SetDeltaTime(Seconds);
		Subsystem->Tick(Seconds);
	};

	Subsystem->PlayFeel(Recipe, FFeelTarget::FromActor(Actor));
	Step(0.2f);
	TestTrue(TEXT("The collision root keeps its scale"), Collision->GetRelativeScale3D().Equals(FVector(1.0), 0.0001));
	TestTrue(TEXT("The visible child gets the punch"), Mesh->GetRelativeScale3D().Equals(FVector(1.5), 0.01));
	TestTrue(TEXT("Non-visual children are left alone"), Pivot->GetRelativeScale3D().Equals(FVector(1.0), 0.0001));
	Step(0.3f);
	TestTrue(TEXT("The child's scale is restored"), Mesh->GetRelativeScale3D().Equals(FVector(1.0), 0.0001));

	// A specific component target is used as it is.
	Subsystem->PlayFeel(Recipe, FFeelTarget::FromComponent(Collision));
	Step(0.2f);
	TestTrue(TEXT("A component target scales that component"), Collision->GetRelativeScale3D().Equals(FVector(1.5), 0.01));
	Step(0.3f);

	// Without collision on the root, the root takes the effect as before.
	Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Subsystem->PlayFeel(Recipe, FFeelTarget::FromActor(Actor));
	Step(0.2f);
	TestTrue(TEXT("A root without collision scales"), Collision->GetRelativeScale3D().Equals(FVector(1.5), 0.01));
	TestTrue(TEXT("Its child is not scaled twice"), Mesh->GetRelativeScale3D().Equals(FVector(1.0), 0.0001));
	Step(0.3f);
	return true;
}

#endif
