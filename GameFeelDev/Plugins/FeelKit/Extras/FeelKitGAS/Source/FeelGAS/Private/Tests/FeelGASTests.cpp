// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "FeelGameplayCue.h"
#include "FeelParameters.h"
#include "FeelRecipe.h"
#include "FeelSubsystem.h"
#include "FeelTags.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "Steps/FeelStep_ScreenFlash.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"

#define FEEL_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelGameplayCueTest, "FeelKit.GAS.GameplayCueNotify", FEEL_TEST_FLAGS)
bool FFeelGameplayCueTest::RunTest(const FString& Parameters)
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
		Track.Duration = 1.0f;
	}

	AActor* Target = World->SpawnActor<AActor>();
	AActor* Instigator = World->SpawnActor<AActor>();
	AActor* Causer = World->SpawnActor<AActor>();

	FGameplayCueParameters CueParameters;
	CueParameters.RawMagnitude = 35.0f;
	CueParameters.NormalizedMagnitude = 0.5f;
	CueParameters.Instigator = Instigator;
	CueParameters.EffectCauser = Causer;
	CueParameters.Location = FVector(1.0, 2.0, 3.0);
	CueParameters.Normal = FVector(0.0, 0.0, 1.0);
	CueParameters.AggregatedSourceTags.AddTag(FeelTags::Screen_Flash);

	FFeelGameplayCueSettings Settings;
	Settings.Recipe = Recipe.Get();
	Settings.RawMagnitudeParameter = TEXT("Damage");
	Settings.NormalizedMagnitudeParameter = TEXT("Strength");
	Settings.bScaleIntensityByMagnitude = true;
	Settings.Intensity = 0.8f;

	const FFeelPlayContext Context = Settings.MakeContext(CueParameters);
	TestEqual(TEXT("Raw magnitude becomes a parameter"), Context.Parameters.FindRef(TEXT("Damage")), 35.0f);
	TestEqual(TEXT("Normalized magnitude becomes a parameter"), Context.Parameters.FindRef(TEXT("Strength")), 0.5f);
	TestTrue(TEXT("Cue instigator is the play instigator"), Context.Instigator == Instigator);
	TestTrue(TEXT("Location kept"), Context.Location.Equals(FVector(1.0, 2.0, 3.0)));
	TestTrue(TEXT("Effect tags become context tags"), Context.ContextTags.HasTagExact(FeelTags::Screen_Flash));

	Settings.bEffectCauserAsInstigator = true;
	TestTrue(TEXT("Effect causer can be the instigator"), Settings.MakeContext(CueParameters).Instigator == Causer);

	UFeelSubsystem* Subsystem = World->GetSubsystem<UFeelSubsystem>();
	const FFeelHandle Handle = Settings.Play(Target, CueParameters);
	if (TestTrue(TEXT("The cue plays the recipe"), Subsystem->IsPlaying(Handle)))
	{
		const FFeelInstance* Instance = Subsystem->GetInstances().FindByPredicate([&Handle](const FFeelInstance& Candidate) { return Candidate.Id == Handle.GetId(); });
		TestTrue(TEXT("Intensity is scaled by the normalized magnitude"), Instance && FMath::IsNearlyEqual(Instance->Intensity, 0.4f));
		TestTrue(TEXT("The cue target is the play target"), Instance && Instance->TargetActor.Get() == Target);
	}

	// The static notify plays on Executed and ignores Removed.
	UFeelGameplayCueNotify* Notify = NewObject<UFeelGameplayCueNotify>(GetTransientPackage());
	Notify->Feel.Recipe = Recipe.Get();
	const int32 Before = Subsystem->GetNumActiveInstances();
	Notify->HandleGameplayCue(Target, EGameplayCueEvent::Removed, CueParameters);
	TestEqual(TEXT("Removed does not play"), Subsystem->GetNumActiveInstances(), Before);
	Notify->HandleGameplayCue(Target, EGameplayCueEvent::Executed, CueParameters);
	TestEqual(TEXT("Executed plays"), Subsystem->GetNumActiveInstances(), Before + 1);
	return true;
}

#endif
