// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "FeelAnimNotifies.h"
#include "FeelEvaluator.h"
#include "FeelFrameOutput.h"
#include "FeelMap.h"
#include "FeelPlaybackClock.h"
#include "FeelRecipe.h"
#include "FeelSettings.h"
#include "FeelSubsystem.h"
#include "FeelTags.h"
#include "FeelTrackLifecycle.h"
#include "FeelTriggerComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/DamageType.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/App.h"
#include "Steps/FeelStep_CameraPunch.h"
#include "Steps/FeelStep_ScalePunch.h"
#include "Steps/FeelStep_ScreenFlash.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#define FEEL_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

namespace FeelInputsTests
{
	/** A minimal game world with a Feel subsystem, destroyed at the end of the scope. */
	class FScopedWorld
	{
	public:
		FScopedWorld()
			: PreviousDeltaTime(FApp::GetDeltaTime())
		{
			World = UWorld::CreateWorld(EWorldType::Game, false);
			FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
			World->BeginPlay();
		}

		~FScopedWorld()
		{
			FApp::SetDeltaTime(PreviousDeltaTime);
			World->RemoveFromRoot();
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}

		UFeelSubsystem* Subsystem() const { return World->GetSubsystem<UFeelSubsystem>(); }

		AActor* SpawnActorWithRoot()
		{
			AActor* Actor = World->SpawnActor<AActor>();
			USceneComponent* Root = NewObject<USceneComponent>(Actor, TEXT("Root"));
			Actor->SetRootComponent(Root);
			Root->RegisterComponent();
			return Actor;
		}

		void Step(float Seconds, int32 Count = 1)
		{
			for (int32 Index = 0; Index < Count; ++Index)
			{
				FApp::SetDeltaTime(Seconds);
				Subsystem()->Tick(Seconds);
			}
		}

		UWorld* World = nullptr;

	private:
		double PreviousDeltaTime = 0.0;
	};

	/** Recipe with one constant, full-opacity flash track. */
	UFeelRecipe* MakeFlashRecipe(float StartTime, float Duration)
	{
		UFeelRecipe* Recipe = NewObject<UFeelRecipe>(GetTransientPackage());
		FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
		UFeelStep_ScreenFlash* Flash = NewObject<UFeelStep_ScreenFlash>(Recipe);
		Flash->MaxOpacity = 1.0f;
		Track.Step = Flash;
		Track.Channel = FeelTags::Screen_Flash;
		Track.StartTime = StartTime;
		Track.Duration = Duration;
		Track.IntensityCurve.GetRichCurve()->Reset();
		return Recipe;
	}

	void AddDamageParameter(UFeelRecipe* Recipe, FName Name)
	{
		FFeelRecipeParameter& Parameter = Recipe->Parameters.AddDefaulted_GetRef();
		Parameter.Name = Name;
		Parameter.MaxValue = 100.0f;
		Parameter.DefaultValue = 0.0f;
		Recipe->Tracks[0].ParameterMappings.AddDefaulted_GetRef().Parameter = Name;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelSustainClockTest, "FeelKit.Sustain.Clock", FEEL_TEST_FLAGS)
bool FFeelSustainClockTest::RunTest(const FString& Parameters)
{
	using namespace FeelInputsTests;

	TStrongObjectPtr<UFeelRecipe> Recipe(MakeFlashRecipe(0.0f, 2.0f));
	FFeelPlaybackClock Clock;
	TestFalse(TEXT("No sustain by default"), FFeelPlaybackClock::HasSustain(*Recipe));
	TestFalse(TEXT("Without sustain time never wraps"), Clock.Advance(*Recipe, 1.5f));
	TestEqual(TEXT("Without sustain time advances"), Clock.RecipeTime, 1.5f, 0.0001f);

	Recipe->bSustain = true;
	Recipe->SustainStart = 0.5f;
	Recipe->SustainEnd = 1.0f;
	Clock = FFeelPlaybackClock();
	TestFalse(TEXT("Inside the region no wrap"), Clock.Advance(*Recipe, 0.8f));
	TestTrue(TEXT("Passing the region end wraps"), Clock.Advance(*Recipe, 0.4f));
	TestEqual(TEXT("Time wraps to the region start plus the overshoot"), Clock.RecipeTime, 0.7f, 0.0001f);
	TestTrue(TEXT("Large steps wrap too"), Clock.Advance(*Recipe, 3.0f));
	TestTrue(TEXT("Wrapped time stays in the region"), Clock.RecipeTime >= 0.5f && Clock.RecipeTime <= 1.0f);

	Clock.Release();
	TestFalse(TEXT("Released time no longer wraps"), Clock.Advance(*Recipe, 1.0f));
	TestTrue(TEXT("Released time continues past the region"), Clock.RecipeTime > 1.0f);

	Recipe->SustainEnd = 0.5f;
	TestFalse(TEXT("An empty region is not a sustain"), FFeelPlaybackClock::HasSustain(*Recipe));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelSustainLifecycleTest, "FeelKit.Sustain.LifecycleRewind", FEEL_TEST_FLAGS)
bool FFeelSustainLifecycleTest::RunTest(const FString& Parameters)
{
	using namespace FeelInputsTests;

	TStrongObjectPtr<UFeelRecipe> Recipe(MakeFlashRecipe(0.6f, 0.2f));
	{
		// A second track that starts before the region and spans it.
		FFeelTrack Spanning = Recipe->Tracks[0];
		Spanning.StartTime = 0.0f;
		Spanning.Duration = 2.0f;
		Recipe->Tracks.Add(Spanning);
	}
	Recipe->bSustain = true;
	Recipe->SustainStart = 0.5f;
	Recipe->SustainEnd = 1.0f;

	FFeelEvalParams Params;
	FFeelTrackLifecycle Lifecycle;
	Lifecycle.Reset(Recipe->Tracks.Num());
	auto MakeContext = [](int32, float) { return FFeelContext(); };

	Lifecycle.Update(*Recipe, 0.7f, Params, MakeContext);
	TestEqual(TEXT("Both tracks run inside the region"), Lifecycle.GetNumRunning(), 2);
	Lifecycle.Update(*Recipe, 1.0f, Params, MakeContext);
	TestEqual(TEXT("The region track ends before the loop"), Lifecycle.GetNumRunning(), 1);

	Lifecycle.Rewind(*Recipe, 0.5f, MakeContext);
	TestEqual(TEXT("The spanning track keeps running across the rewind"), Lifecycle.GetNumRunning(), 1);
	Lifecycle.Update(*Recipe, 0.65f, Params, MakeContext);
	TestEqual(TEXT("The region track starts again on the next loop"), Lifecycle.GetNumRunning(), 2);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelSustainRuntimeTest, "FeelKit.Runtime.SustainAndRelease", FEEL_TEST_FLAGS)
bool FFeelSustainRuntimeTest::RunTest(const FString& Parameters)
{
	using namespace FeelInputsTests;

	FScopedWorld TestWorld;
	UFeelSubsystem* Subsystem = TestWorld.Subsystem();
	TStrongObjectPtr<UFeelRecipe> Recipe(MakeFlashRecipe(0.0f, 1.5f));
	Recipe->bSustain = true;
	Recipe->SustainStart = 0.2f;
	Recipe->SustainEnd = 0.6f;

	int32 StartedCount = 0;
	int32 FinishedNormally = 0;
	const FDelegateHandle StartedHandle = Subsystem->OnFeelStarted.AddLambda([&StartedCount](FFeelHandle, UFeelRecipe*) { ++StartedCount; });
	const FDelegateHandle FinishedHandle = Subsystem->OnFeelFinished.AddLambda([&FinishedNormally](FFeelHandle, UFeelRecipe*, bool bInterrupted) { FinishedNormally += bInterrupted ? 0 : 1; });

	const FFeelHandle Handle = Subsystem->PlayFeel(Recipe.Get(), FFeelTarget());
	TestEqual(TEXT("Started delegate fires"), StartedCount, 1);

	TestWorld.Step(0.1f, 30);
	TestTrue(TEXT("A sustained recipe keeps playing long past its length"), Subsystem->IsPlaying(Handle));
	TestTrue(TEXT("It keeps producing output"), Subsystem->GetScreenOutput().FlashAlpha > 0.0f);

	Subsystem->ReleaseFeel(Handle);
	TestWorld.Step(0.1f, 20);
	TestFalse(TEXT("After release it plays to the end and finishes"), Subsystem->IsPlaying(Handle));
	TestEqual(TEXT("Finished delegate reports a normal finish"), FinishedNormally, 1);

	Subsystem->OnFeelStarted.Remove(StartedHandle);
	Subsystem->OnFeelFinished.Remove(FinishedHandle);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelLiveParameterTest, "FeelKit.Runtime.SetFeelParameterAndGlobalScale", FEEL_TEST_FLAGS)
bool FFeelLiveParameterTest::RunTest(const FString& Parameters)
{
	using namespace FeelInputsTests;

	FScopedWorld TestWorld;
	UFeelSubsystem* Subsystem = TestWorld.Subsystem();
	TStrongObjectPtr<UFeelRecipe> Recipe(MakeFlashRecipe(0.0f, 2.0f));
	const FName Strength(TEXT("Strength"));
	AddDamageParameter(Recipe.Get(), Strength);

	FFeelPlayContext Context;
	Context.Parameters.Add(Strength, 0.0f);
	const FFeelHandle Handle = Subsystem->PlayFeel(Recipe.Get(), FFeelTarget(), 1.0f, Context);
	TestWorld.Step(0.1f);
	TestEqual(TEXT("Starts at the passed value"), Subsystem->GetScreenOutput().FlashAlpha, 0.0f, 0.001f);

	TestTrue(TEXT("Set Feel Parameter succeeds while playing"), Subsystem->SetFeelParameter(Handle, Strength, 100.0f));
	TestWorld.Step(0.1f);
	TestEqual(TEXT("The new value applies on the next frame"), Subsystem->GetScreenOutput().FlashAlpha, 1.0f, 0.001f);

	IConsoleVariable* GlobalScale = IConsoleManager::Get().FindConsoleVariable(TEXT("feel.GlobalScale"));
	if (TestNotNull(TEXT("feel.GlobalScale exists"), GlobalScale))
	{
		GlobalScale->Set(0.5f, ECVF_SetByCode);
		TestWorld.Step(0.1f);
		TestEqual(TEXT("feel.GlobalScale scales every recipe"), Subsystem->GetScreenOutput().FlashAlpha, 0.5f, 0.001f);
		GlobalScale->Set(1.0f, ECVF_SetByCode);
	}

	Subsystem->StopAllFeel();
	TestFalse(TEXT("Set Feel Parameter fails once stopped"), Subsystem->SetFeelParameter(Handle, Strength, 1.0f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelAccumulatorTest, "FeelKit.Runtime.Accumulators", FEEL_TEST_FLAGS)
bool FFeelAccumulatorTest::RunTest(const FString& Parameters)
{
	using namespace FeelInputsTests;

	UFeelSettings* Settings = GetMutableDefault<UFeelSettings>();
	const TArray<FFeelAccumulatorDefinition> SavedAccumulators = Settings->Accumulators;
	const FName Streak(TEXT("FeelKitTestStreak"));
	FFeelAccumulatorDefinition& Definition = Settings->Accumulators.AddDefaulted_GetRef();
	Definition.Name = Streak;
	Definition.MaxValue = 1.0f;
	Definition.DecayPerSecond = 1.0f;
	Definition.DecayDelay = 0.25f;

	{
		FScopedWorld TestWorld;
		UFeelSubsystem* Subsystem = TestWorld.Subsystem();
		AActor* ActorA = TestWorld.SpawnActorWithRoot();
		AActor* ActorB = TestWorld.SpawnActorWithRoot();

		Subsystem->AddToAccumulator(Streak, 0.4f, ActorA);
		Subsystem->AddToAccumulator(Streak, 0.4f, ActorA);
		TestEqual(TEXT("Additions add up"), Subsystem->GetAccumulator(Streak, ActorA), 0.8f, 0.0001f);
		Subsystem->AddToAccumulator(Streak, 0.9f, ActorA);
		TestEqual(TEXT("Values are clamped to the max"), Subsystem->GetAccumulator(Streak, ActorA), 1.0f, 0.0001f);
		TestEqual(TEXT("Values are per actor"), Subsystem->GetAccumulator(Streak, ActorB), 0.0f);
		TestEqual(TEXT("Actor values do not change the global value"), Subsystem->GetAccumulator(Streak), 0.0f);

		TestWorld.Step(0.1f, 2);
		TestEqual(TEXT("No decay during the delay"), Subsystem->GetAccumulator(Streak, ActorA), 1.0f, 0.0001f);
		TestWorld.Step(0.1f, 5);
		TestTrue(TEXT("Decays after the delay"), Subsystem->GetAccumulator(Streak, ActorA) < 0.8f && Subsystem->GetAccumulator(Streak, ActorA) > 0.2f);

		// A recipe parameter reading the accumulator follows it while playing.
		TStrongObjectPtr<UFeelRecipe> Recipe(MakeFlashRecipe(0.0f, 5.0f));
		const FName StreakParameter(TEXT("StreakValue"));
		FFeelRecipeParameter& Parameter = Recipe->Parameters.AddDefaulted_GetRef();
		Parameter.Name = StreakParameter;
		Parameter.MaxValue = 1.0f;
		Parameter.Accumulator = Streak;
		Recipe->Tracks[0].ParameterMappings.AddDefaulted_GetRef().Parameter = StreakParameter;

		Subsystem->SetAccumulator(Streak, 0.0f, ActorB);
		Subsystem->PlayFeel(Recipe.Get(), FFeelTarget::FromActor(ActorB));
		TestWorld.Step(0.01f);
		TestEqual(TEXT("Parameter reads the target's accumulator"), Subsystem->GetScreenOutput().FlashAlpha, 0.0f, 0.02f);
		Subsystem->SetAccumulator(Streak, 0.6f, ActorB);
		TestWorld.Step(0.01f);
		TestEqual(TEXT("Parameter follows the accumulator while playing"), Subsystem->GetScreenOutput().FlashAlpha, 0.6f, 0.02f);
	}

	Settings->Accumulators = SavedAccumulators;
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelMapMatchingTest, "FeelKit.FeelMap.Matching", FEEL_TEST_FLAGS)
bool FFeelMapMatchingTest::RunTest(const FString& Parameters)
{
	using namespace FeelInputsTests;

	TStrongObjectPtr<UFeelRecipe> General(MakeFlashRecipe(0.0f, 1.0f));
	TStrongObjectPtr<UFeelRecipe> Specific(MakeFlashRecipe(0.0f, 1.0f));
	TStrongObjectPtr<UFeelRecipe> Tagged(MakeFlashRecipe(0.0f, 1.0f));
	TStrongObjectPtr<UFeelRecipe> Preferred(MakeFlashRecipe(0.0f, 1.0f));
	TStrongObjectPtr<UFeelRecipe> Override(MakeFlashRecipe(0.0f, 1.0f));

	TStrongObjectPtr<UFeelMap> ProjectMap(NewObject<UFeelMap>(GetTransientPackage()));
	auto AddEntry = [](UFeelMap* Map, const FGameplayTag& Event, UFeelRecipe* Recipe, const FGameplayTag& RequiredTag, int32 Priority)
	{
		FFeelMapEntry& Entry = Map->Entries.AddDefaulted_GetRef();
		Entry.Event = Event;
		Entry.Recipe = Recipe;
		Entry.Priority = Priority;
		if (RequiredTag.IsValid())
		{
			Entry.RequiredTags.AddTag(RequiredTag);
		}
	};
	const FGameplayTag CameraParent = FGameplayTag::RequestGameplayTag(TEXT("Feel.Camera"));
	const FGameplayTag ScreenParent = FGameplayTag::RequestGameplayTag(TEXT("Feel.Screen"));
	AddEntry(ProjectMap.Get(), CameraParent, General.Get(), FGameplayTag(), 0);
	AddEntry(ProjectMap.Get(), ScreenParent, General.Get(), FGameplayTag(), 0);
	AddEntry(ProjectMap.Get(), FeelTags::Camera_Shake, Specific.Get(), FGameplayTag(), 0);
	AddEntry(ProjectMap.Get(), FeelTags::Camera_Shake, Tagged.Get(), FeelTags::Screen_Flash, 0);
	AddEntry(ProjectMap.Get(), FeelTags::Camera_Motion, General.Get(), FGameplayTag(), 0);
	AddEntry(ProjectMap.Get(), FeelTags::Camera_Motion, Preferred.Get(), FGameplayTag(), 5);

	TArray<const UFeelMap*> Maps = { ProjectMap.Get() };
	FGameplayTagContainer NoTags;
	FGameplayTagContainer FlashTags;
	FlashTags.AddTag(FeelTags::Screen_Flash);

	auto Find = [&Maps](const FGameplayTag& Event, const FGameplayTagContainer& Tags) -> UFeelRecipe*
	{
		const FFeelMapEntry* Entry = UFeelMap::FindBestEntry(Maps, Event, Tags);
		return Entry ? Entry->Recipe.Get() : nullptr;
	};

	TestTrue(TEXT("The exact event row wins over its parent"), Find(FeelTags::Camera_Shake, NoTags) == Specific.Get());
	TestTrue(TEXT("A row whose required tags are present wins"), Find(FeelTags::Camera_Shake, FlashTags) == Tagged.Get());
	TestTrue(TEXT("Higher priority breaks ties"), Find(FeelTags::Camera_Motion, NoTags) == Preferred.Get());
	TestTrue(TEXT("A parent row answers a child event without its own row"), Find(FeelTags::Screen_Distortion, NoTags) == General.Get());
	TestNull(TEXT("Unrelated events match nothing"), Find(FeelTags::Haptics, NoTags));

	// Maps earlier in the list win ties.
	TStrongObjectPtr<UFeelMap> ActorMap(NewObject<UFeelMap>(GetTransientPackage()));
	AddEntry(ActorMap.Get(), FeelTags::Camera_Shake, Override.Get(), FGameplayTag(), 0);
	Maps.Insert(ActorMap.Get(), 0);
	TestTrue(TEXT("An earlier map wins an equal match"), Find(FeelTags::Camera_Shake, NoTags) == Override.Get());

#if WITH_EDITOR
	FDataValidationContext Validation;
	ProjectMap->Entries.AddDefaulted();
	ProjectMap->IsDataValid(Validation);
	TestEqual(TEXT("A row without event and recipe is two errors"), Validation.GetNumErrors(), 2);
#endif

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelEventsAndTriggersTest, "FeelKit.Runtime.EventsAndTriggers", FEEL_TEST_FLAGS)
bool FFeelEventsAndTriggersTest::RunTest(const FString& Parameters)
{
	using namespace FeelInputsTests;

	FScopedWorld TestWorld;
	UFeelSubsystem* Subsystem = TestWorld.Subsystem();
	AActor* Actor = TestWorld.SpawnActorWithRoot();

	// A punch recipe scaled by the event value.
	TStrongObjectPtr<UFeelRecipe> Recipe(NewObject<UFeelRecipe>(GetTransientPackage()));
	{
		FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
		UFeelStep_ScalePunch* Punch = NewObject<UFeelStep_ScalePunch>(Recipe.Get());
		Punch->Amount = FVector(1.0);
		Punch->Bounces = 0;
		Track.Step = Punch;
		Track.Duration = 0.4f;
		Track.IntensityCurve.GetRichCurve()->Reset();
		FFeelRecipeParameter& Parameter = Recipe->Parameters.AddDefaulted_GetRef();
		Parameter.Name = TEXT("Amount");
		Parameter.MaxValue = 100.0f;
		Track.ParameterMappings.AddDefaulted_GetRef().Parameter = Parameter.Name;
	}

	UFeelTriggerComponent* Trigger = NewObject<UFeelTriggerComponent>(Actor);
	FFeelTriggerEntry& Entry = Trigger->Triggers.AddDefaulted_GetRef();
	Entry.Event = EFeelTriggerEvent::TakeAnyDamage;
	Entry.Recipe = Recipe.Get();
	Entry.ValueParameter = TEXT("Amount");
	Entry.bTargetSkeletalMesh = false;
	Trigger->RegisterComponent();

	// The bare test world has no game mode, so actors do not begin play on their own; in a game they do.
	Actor->DispatchBeginPlay();
	TestTrue(TEXT("The trigger binds to the owner's damage event on begin play"), Actor->OnTakeAnyDamage.IsBound());

	UGameplayStatics::ApplyDamage(Actor, 50.0f, nullptr, nullptr, UDamageType::StaticClass());
	TestEqual(TEXT("Damage starts the trigger's recipe"), Subsystem->GetNumActiveInstances(), 1);
	TestWorld.Step(0.2f);
	TestTrue(TEXT("The damage amount reaches the parameter (50 of 100 gives half the punch)"), Actor->GetRootComponent()->GetRelativeScale3D().Equals(FVector(1.5), 0.02));
	TestWorld.Step(0.3f);

	// Send Feel Event through the trigger component's map.
	TStrongObjectPtr<UFeelMap> Map(NewObject<UFeelMap>(GetTransientPackage()));
	FFeelMapEntry& MapEntry = Map->Entries.AddDefaulted_GetRef();
	MapEntry.Event = FeelTags::Actor_Transform;
	MapEntry.Recipe = Recipe.Get();
	MapEntry.IntensityScale = 0.5f;
	Trigger->FeelMaps.Add(Map.Get());

	FFeelPlayContext Context;
	Context.Parameters.Add(TEXT("Amount"), 100.0f);
	TestTrue(TEXT("A mapped event plays"), Subsystem->SendFeelEvent(FeelTags::Actor_Transform, FFeelTarget::FromActor(Actor), 1.0f, Context).IsValid());
	TestWorld.Step(0.2f);
	TestTrue(TEXT("The row's intensity scale applies"), Actor->GetRootComponent()->GetRelativeScale3D().Equals(FVector(1.5), 0.02));
	TestWorld.Step(0.3f);
	TestFalse(TEXT("An unmapped event plays nothing"), Subsystem->SendFeelEvent(FeelTags::Screen_Flash, FFeelTarget::FromActor(Actor)).IsValid());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelAnimNotifyTest, "FeelKit.Runtime.AnimNotifies", FEEL_TEST_FLAGS)
bool FFeelAnimNotifyTest::RunTest(const FString& Parameters)
{
	using namespace FeelInputsTests;

	FScopedWorld TestWorld;
	UFeelSubsystem* Subsystem = TestWorld.Subsystem();
	AActor* Actor = TestWorld.SpawnActorWithRoot();
	USkeletalMeshComponent* Mesh = NewObject<USkeletalMeshComponent>(Actor);
	Mesh->SetupAttachment(Actor->GetRootComponent());
	Mesh->RegisterComponent();

	TStrongObjectPtr<UFeelRecipe> Recipe(MakeFlashRecipe(0.0f, 1.0f));
	TStrongObjectPtr<UAnimNotify_PlayFeel> Notify(NewObject<UAnimNotify_PlayFeel>(GetTransientPackage()));
	Notify->Recipe = Recipe.Get();
	Notify->Notify(Mesh, nullptr, FAnimNotifyEventReference());
	TestEqual(TEXT("Play Feel notify starts the recipe"), Subsystem->GetNumActiveInstances(), 1);
	Subsystem->StopAllFeel();
	TestWorld.Step(0.01f);

	TStrongObjectPtr<UFeelRecipe> Sustained(MakeFlashRecipe(0.0f, 1.0f));
	Sustained->bSustain = true;
	Sustained->SustainStart = 0.1f;
	Sustained->SustainEnd = 0.5f;
	TStrongObjectPtr<UAnimNotifyState_PlayFeel> Window(NewObject<UAnimNotifyState_PlayFeel>(GetTransientPackage()));
	Window->Recipe = Sustained.Get();
	Window->NotifyBegin(Mesh, nullptr, 2.0f, FAnimNotifyEventReference());
	TestWorld.Step(0.1f, 20);
	TestEqual(TEXT("A sustained recipe loops for the whole window"), Subsystem->GetNumActiveInstances(), 1);
	Window->NotifyEnd(Mesh, nullptr, FAnimNotifyEventReference());
	TestWorld.Step(0.1f, 12);
	TestEqual(TEXT("Ending the window releases it and it finishes"), Subsystem->GetNumActiveInstances(), 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelDirectionTest, "FeelKit.Steps.CameraPunchDirection", FEEL_TEST_FLAGS)
bool FFeelDirectionTest::RunTest(const FString& Parameters)
{
	TStrongObjectPtr<UFeelStep_CameraPunch> Punch(NewObject<UFeelStep_CameraPunch>(GetTransientPackage()));
	Punch->LocationPunch = FVector(-10.0, 0.0, 0.0);
	Punch->Shape = EFeelMotionShape::Smooth;

	FFeelStepEvalContext Context;
	Context.Duration = 1.0f;
	Context.LocalTime = 0.5f;
	Context.Alpha = 0.5f;
	Context.Intensity = 1.0f;
	Context.ViewDirection = FVector(0.0, 1.0, 0.0);
	Context.ViewDirectionFromLocation = FVector(0.0, 0.0, 1.0);

	FFeelOutputAccumulator Settings;
	Punch->Evaluate(Context, Settings);
	TestTrue(TEXT("Step Settings ignores the play direction"), FMath::Abs(Settings.Output.CameraLocationOffset.Y) < 0.001 && Settings.Output.CameraLocationOffset.X < 0.0);

	Punch->DirectionSource = EFeelDirectionSource::PlayDirection;
	FFeelOutputAccumulator PlayDirection;
	Punch->Evaluate(Context, PlayDirection);
	TestTrue(TEXT("Play Direction points the punch along the view direction"), PlayDirection.Output.CameraLocationOffset.Y > 0.0 && FMath::Abs(PlayDirection.Output.CameraLocationOffset.X) < 0.001);
	TestEqual(TEXT("Play Direction keeps the punch strength"), PlayDirection.Output.CameraLocationOffset.Size(), Settings.Output.CameraLocationOffset.Size(), 0.01);

	Punch->DirectionSource = EFeelDirectionSource::TowardPlayLocation;
	FFeelOutputAccumulator Toward;
	Punch->Evaluate(Context, Toward);
	TestTrue(TEXT("Toward Play Location reverses the away direction"), Toward.Output.CameraLocationOffset.Z < 0.0);

	Context.ViewDirection = FVector::ZeroVector;
	Punch->DirectionSource = EFeelDirectionSource::PlayDirection;
	FFeelOutputAccumulator Fallback;
	Punch->Evaluate(Context, Fallback);
	TestTrue(TEXT("Without a play direction the step settings apply"), Fallback.Output.CameraLocationOffset.Equals(Settings.Output.CameraLocationOffset, 0.001));

	return true;
}

#if WITH_EDITOR
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelSustainValidationTest, "FeelKit.Validation.SustainAndAccumulators", FEEL_TEST_FLAGS)
bool FFeelSustainValidationTest::RunTest(const FString& Parameters)
{
	using namespace FeelInputsTests;

	TStrongObjectPtr<UFeelRecipe> Recipe(MakeFlashRecipe(0.0f, 1.0f));
	Recipe->bSustain = true;
	Recipe->SustainStart = 0.5f;
	Recipe->SustainEnd = 0.5f;

	FDataValidationContext Empty;
	Recipe->IsDataValid(Empty);
	TestEqual(TEXT("An empty sustain region is an error"), Empty.GetNumErrors(), 1);

	Recipe->SustainEnd = 3.0f;
	FDataValidationContext PastEnd;
	Recipe->IsDataValid(PastEnd);
	TestEqual(TEXT("A region past the last track is a warning"), PastEnd.GetNumWarnings(), 1);

	Recipe->bSustain = false;
	FFeelRecipeParameter& Parameter = Recipe->Parameters.AddDefaulted_GetRef();
	Parameter.Name = TEXT("Level");
	Parameter.Accumulator = TEXT("FeelKitTestUndefinedAccumulator");
	FDataValidationContext Unknown;
	Recipe->IsDataValid(Unknown);
	TestEqual(TEXT("An undefined accumulator is a warning"), Unknown.GetNumWarnings(), 1);

	return true;
}
#endif

#undef FEEL_TEST_FLAGS

#endif // WITH_DEV_AUTOMATION_TESTS
