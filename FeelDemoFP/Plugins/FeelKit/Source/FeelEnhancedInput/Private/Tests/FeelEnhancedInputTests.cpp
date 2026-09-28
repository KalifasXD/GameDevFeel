// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "FeelInputComponent.h"
#include "FeelRecipe.h"
#include "FeelSubsystem.h"
#include "FeelTags.h"
#include "GameFramework/Actor.h"
#include "Misc/App.h"
#include "Steps/FeelStep_ScreenFlash.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"

#define FEEL_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelInputComponentTest, "FeelKit.EnhancedInput.InputComponent", FEEL_TEST_FLAGS)
bool FFeelInputComponentTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	World->BeginPlay();
	ON_SCOPE_EXIT
	{
		World->RemoveFromRoot();
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	};

	TStrongObjectPtr<UFeelRecipe> Recipe(NewObject<UFeelRecipe>(GetTransientPackage()));
	{
		FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
		Track.Step = NewObject<UFeelStep_ScreenFlash>(Recipe.Get());
		Track.Channel = FeelTags::Screen_Flash;
		Track.Duration = 5.0f;
	}

	AActor* Owner = World->SpawnActor<AActor>();
	UFeelInputComponent* Component = NewObject<UFeelInputComponent>(Owner);
	{
		FFeelInputBinding& Hold = Component->Bindings.AddDefaulted_GetRef();
		Hold.Recipe = Recipe.Get();
		Hold.PlayOn = ETriggerEvent::Started;
		Hold.bEndWhenInputEnds = true;
		Hold.bScaleIntensityByValue = true;
		Hold.ValueParameter = TEXT("Pressure");
	}
	{
		FFeelInputBinding& Repeat = Component->Bindings.AddDefaulted_GetRef();
		Repeat.Recipe = Recipe.Get();
		Repeat.PlayOn = ETriggerEvent::Triggered;
		Repeat.bOncePerPress = true;
	}
	Component->RegisterComponent();

	UFeelSubsystem* Subsystem = World->GetSubsystem<UFeelSubsystem>();

	Component->HandleInput(0, ETriggerEvent::Triggered, 1.0f);
	TestFalse(TEXT("Other trigger events do not play"), Component->GetActiveHandle(0).IsValid());

	Component->HandleInput(0, ETriggerEvent::Started, 0.5f);
	const FFeelHandle Held = Component->GetActiveHandle(0);
	if (TestTrue(TEXT("Started plays"), Subsystem->IsPlaying(Held)))
	{
		const FFeelInstance* Instance = Subsystem->GetInstances().FindByPredicate([&Held](const FFeelInstance& Candidate) { return Candidate.Id == Held.GetId(); });
		TestTrue(TEXT("Intensity scaled by the action value"), Instance && FMath::IsNearlyEqual(Instance->Intensity, 0.5f));
		TestTrue(TEXT("Action value sent as a parameter"), Instance && FMath::IsNearlyEqual(Instance->ParameterValues.FindRef(TEXT("Pressure")), 0.5f));
		TestTrue(TEXT("Plays target the owner"), Instance && Instance->TargetActor.Get() == Owner);
	}

	Component->HandleInput(0, ETriggerEvent::Completed, 0.0f);
	TestFalse(TEXT("The play ends with the input"), Component->GetActiveHandle(0).IsValid());
	FApp::SetDeltaTime(0.5f);
	Subsystem->Tick(0.5f);
	TestFalse(TEXT("A non-sustained recipe stops with a blend out"), Subsystem->IsPlaying(Held));

	const int32 Before = Subsystem->GetNumActiveInstances();
	Component->HandleInput(1, ETriggerEvent::Triggered, 1.0f);
	Component->HandleInput(1, ETriggerEvent::Triggered, 1.0f);
	Component->HandleInput(1, ETriggerEvent::Triggered, 1.0f);
	TestEqual(TEXT("Once per press: repeated triggers play once"), Subsystem->GetNumActiveInstances(), Before + 1);
	Component->HandleInput(1, ETriggerEvent::Completed, 0.0f);
	Component->HandleInput(1, ETriggerEvent::Triggered, 1.0f);
	TestEqual(TEXT("A new press plays again"), Subsystem->GetNumActiveInstances(), Before + 2);

	Component->HandleInput(7, ETriggerEvent::Started, 1.0f);
	TestEqual(TEXT("An invalid binding index is ignored"), Subsystem->GetNumActiveInstances(), Before + 2);
	return true;
}

#endif
