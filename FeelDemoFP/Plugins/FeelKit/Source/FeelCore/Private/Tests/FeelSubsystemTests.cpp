// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "FeelBlueprintLibrary.h"
#include "FeelRecipe.h"
#include "FeelSwitch.h"
#include "FeelSubsystem.h"
#include "GameFramework/Actor.h"
#include "GameFramework/WorldSettings.h"
#include "HAL/IConsoleManager.h"
#include "Misc/App.h"
#include "Steps/FeelStep_Hitstop.h"
#include "Steps/FeelStep_ScalePunch.h"
#include "Steps/FeelStep_ScreenFlash.h"
#include "UObject/Package.h"

#define FEEL_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

namespace FeelSubsystemTests
{
	/** A minimal game world with a Feel subsystem, destroyed at the end of the scope. */
	class FScopedTestWorld
	{
	public:
		FScopedTestWorld()
			: PreviousDeltaTime(FApp::GetDeltaTime())
		{
			World = UWorld::CreateWorld(EWorldType::Game, false);
			FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
			World->BeginPlay();
		}

		~FScopedTestWorld()
		{
			FApp::SetDeltaTime(PreviousDeltaTime);
			World->RemoveFromRoot();
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}

		UFeelSubsystem* GetSubsystem() const
		{
			return World->GetSubsystem<UFeelSubsystem>();
		}

		AActor* SpawnTarget(const FVector& Scale = FVector::OneVector)
		{
			AActor* Actor = World->SpawnActor<AActor>();
			USceneComponent* Root = NewObject<USceneComponent>(Actor, TEXT("Root"));
			Actor->SetRootComponent(Root);
			Root->RegisterComponent();
			Root->SetRelativeScale3D(Scale);
			return Actor;
		}

		/** Advances the subsystem by Seconds of real time. */
		void Step(float Seconds)
		{
			FApp::SetDeltaTime(Seconds);
			if (UFeelSubsystem* Subsystem = GetSubsystem())
			{
				Subsystem->Tick(Seconds);
			}
		}

		UWorld* World = nullptr;

	private:
		double PreviousDeltaTime = 0.0;
	};

	/** Adds a track with a constant intensity curve. */
	void AddTrack(UFeelRecipe* Recipe, UFeelStep* Step, float StartTime, float Duration)
	{
		FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
		Track.Step = Step;
		Track.StartTime = StartTime;
		Track.Duration = Duration;
		Track.IntensityCurve.GetRichCurve()->Reset();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelSubsystemTickTest, "FeelKit.Runtime.TicksOnlyWhenActive", FEEL_TEST_FLAGS)
bool FFeelSubsystemTickTest::RunTest(const FString& Parameters)
{
	using namespace FeelSubsystemTests;

	FScopedTestWorld TestWorld;
	UFeelSubsystem* Subsystem = TestWorld.GetSubsystem();
	if (!TestNotNull(TEXT("Subsystem exists in game worlds"), Subsystem))
	{
		return false;
	}
	TestFalse(TEXT("Does not tick without instances"), Subsystem->IsTickable());

	UFeelRecipe* Recipe = NewObject<UFeelRecipe>(GetTransientPackage());
	AddTrack(Recipe, NewObject<UFeelStep_ScreenFlash>(Recipe), 0.0f, 0.25f);

	const FFeelHandle Handle = Subsystem->PlayFeel(Recipe, FFeelTarget(), 1.0f);
	TestTrue(TEXT("Play returns a valid handle"), Handle.IsValid());
	TestTrue(TEXT("Ticks while playing"), Subsystem->IsTickable());
	TestTrue(TEXT("Instance is playing"), Subsystem->IsPlaying(Handle));

	TestWorld.Step(0.1f);
	TestTrue(TEXT("Flash reaches the screen output"), Subsystem->GetScreenOutput().FlashAlpha > 0.0f);

	TestWorld.Step(0.1f);
	TestWorld.Step(0.1f);
	TestFalse(TEXT("Instance finishes after the recipe duration"), Subsystem->IsPlaying(Handle));
	TestEqual(TEXT("No instances remain"), Subsystem->GetNumActiveInstances(), 0);
	TestEqual(TEXT("Screen output is cleared"), Subsystem->GetScreenOutput().FlashAlpha, 0.0f);
	TestFalse(TEXT("Stops ticking once idle"), Subsystem->IsTickable());

	Recipe->Cooldown = 10.0f;
	TestTrue(TEXT("Play with cooldown starts"), Subsystem->PlayFeel(Recipe, FFeelTarget()).IsValid());
	TestFalse(TEXT("Second play during cooldown is refused"), Subsystem->PlayFeel(Recipe, FFeelTarget()).IsValid());
	Subsystem->StopAllFeel();
	TestWorld.Step(0.016f);
	TestFalse(TEXT("Idle again after StopAllFeel"), Subsystem->IsTickable());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelSubsystemHitstopTest, "FeelKit.Runtime.HitstopRestore", FEEL_TEST_FLAGS)
bool FFeelSubsystemHitstopTest::RunTest(const FString& Parameters)
{
	using namespace FeelSubsystemTests;

	FScopedTestWorld TestWorld;
	UFeelSubsystem* Subsystem = TestWorld.GetSubsystem();
	AWorldSettings* WorldSettings = TestWorld.World->GetWorldSettings();
	if (!TestNotNull(TEXT("Subsystem"), Subsystem) || !TestNotNull(TEXT("World settings"), WorldSettings))
	{
		return false;
	}

	WorldSettings->SetTimeDilation(0.8f);
	AActor* Target = TestWorld.SpawnTarget();
	Target->CustomTimeDilation = 0.9f;

	UFeelRecipe* Recipe = NewObject<UFeelRecipe>(GetTransientPackage());
	UFeelStep_GlobalHitstop* GlobalHitstop = NewObject<UFeelStep_GlobalHitstop>(Recipe);
	GlobalHitstop->TimeDilation = 0.05f;
	AddTrack(Recipe, GlobalHitstop, 0.0f, 0.25f);
	UFeelStep_ActorHitstop* ActorHitstop = NewObject<UFeelStep_ActorHitstop>(Recipe);
	ActorHitstop->TimeDilation = 0.1f;
	AddTrack(Recipe, ActorHitstop, 0.0f, 0.25f);

	// Finish.
	FFeelHandle Handle = Subsystem->PlayFeel(Recipe, FFeelTarget::FromActor(Target));
	TestWorld.Step(0.1f);
	TestEqual(TEXT("Global hitstop applies"), WorldSettings->TimeDilation, 0.05f);
	TestEqual(TEXT("Actor hitstop applies"), Target->CustomTimeDilation, 0.1f);

	TestWorld.Step(0.1f);
	TestWorld.Step(0.1f);
	TestFalse(TEXT("Recipe ends on real time despite the global hitstop"), Subsystem->IsPlaying(Handle));
	TestEqual(TEXT("Global dilation restored after the recipe ends"), WorldSettings->TimeDilation, 0.8f);
	TestEqual(TEXT("Actor dilation restored after the recipe ends"), Target->CustomTimeDilation, 0.9f);

	// Stop in the middle of the hitstop.
	Handle = Subsystem->PlayFeel(Recipe, FFeelTarget::FromActor(Target));
	TestWorld.Step(0.05f);
	Subsystem->StopFeel(Handle, false);
	TestWorld.Step(0.016f);
	TestEqual(TEXT("Global dilation restored on stop"), WorldSettings->TimeDilation, 0.8f);
	TestEqual(TEXT("Actor dilation restored on stop"), Target->CustomTimeDilation, 0.9f);

	// Target destroyed in the middle of the hitstop.
	Handle = Subsystem->PlayFeel(Recipe, FFeelTarget::FromActor(Target));
	TestWorld.Step(0.05f);
	Target->Destroy();
	TestWorld.Step(0.016f);
	TestFalse(TEXT("Instance ends when its target is destroyed"), Subsystem->IsPlaying(Handle));
	TestEqual(TEXT("Global dilation restored when the target is destroyed"), WorldSettings->TimeDilation, 0.8f);
	TestFalse(TEXT("Subsystem goes idle"), Subsystem->IsTickable());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelSubsystemScaleTest, "FeelKit.Runtime.ScalePunchRestore", FEEL_TEST_FLAGS)
bool FFeelSubsystemScaleTest::RunTest(const FString& Parameters)
{
	using namespace FeelSubsystemTests;

	FScopedTestWorld TestWorld;
	UFeelSubsystem* Subsystem = TestWorld.GetSubsystem();
	if (!TestNotNull(TEXT("Subsystem"), Subsystem))
	{
		return false;
	}

	AActor* Target = TestWorld.SpawnTarget(FVector(2.0));
	USceneComponent* Root = Target->GetRootComponent();

	UFeelRecipe* Recipe = NewObject<UFeelRecipe>(GetTransientPackage());
	UFeelStep_ScalePunch* Punch = NewObject<UFeelStep_ScalePunch>(Recipe);
	Punch->Amount = FVector(0.5);
	Punch->Bounces = 0;
	AddTrack(Recipe, Punch, 0.0f, 0.4f);

	Subsystem->PlayFeel(Recipe, FFeelTarget::FromActor(Target));
	TestWorld.Step(0.2f);
	TestTrue(TEXT("Scale punch peaks relative to the base scale"), Root->GetRelativeScale3D().Equals(FVector(3.0), 0.01));

	TestWorld.Step(0.3f);
	TestTrue(TEXT("Base scale restored after the punch"), Root->GetRelativeScale3D().Equals(FVector(2.0), 0.0001));
	TestFalse(TEXT("Subsystem goes idle"), Subsystem->IsTickable());

	return true;
}

/** The Feel Switch nodes and actor: off stops FeelKit and restores time, the actor applies and restores its start state. */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelSwitchTest, "FeelKit.Runtime.FeelSwitch", FEEL_TEST_FLAGS)
bool FFeelSwitchTest::RunTest(const FString& Parameters)
{
	using namespace FeelSubsystemTests;

	FScopedTestWorld TestWorld;
	UFeelSubsystem* Subsystem = TestWorld.GetSubsystem();
	AWorldSettings* WorldSettings = TestWorld.World->GetWorldSettings();
	if (!TestNotNull(TEXT("Subsystem"), Subsystem) || !TestNotNull(TEXT("World settings"), WorldSettings))
	{
		return false;
	}
	UFeelBlueprintLibrary::SetFeelEnabled(true);
	WorldSettings->SetTimeDilation(1.0f);

	// Nodes.
	UFeelRecipe* Recipe = NewObject<UFeelRecipe>(GetTransientPackage());
	AddTrack(Recipe, NewObject<UFeelStep_GlobalHitstop>(Recipe), 0.0f, 1.0f);
	const FFeelHandle Handle = Subsystem->PlayFeel(Recipe, FFeelTarget());
	TestWorld.Step(0.1f);
	TestTrue(TEXT("Hitstop is active"), WorldSettings->TimeDilation < 1.0f);

	TestFalse(TEXT("Toggle Feel turns it off"), UFeelBlueprintLibrary::ToggleFeel());
	TestFalse(TEXT("Is Feel Enabled follows"), UFeelBlueprintLibrary::IsFeelEnabled());
	TestWorld.Step(0.1f);
	TestFalse(TEXT("Off stops playback"), Subsystem->IsPlaying(Handle));
	TestEqual(TEXT("Off restores time"), WorldSettings->TimeDilation, 1.0f);
	TestFalse(TEXT("Nothing new plays while off"), Subsystem->PlayFeel(Recipe, FFeelTarget()).IsValid());
	TestTrue(TEXT("Toggle Feel turns it on again"), UFeelBlueprintLibrary::ToggleFeel());
	TestTrue(TEXT("Plays again"), Subsystem->PlayFeel(Recipe, FFeelTarget()).IsValid());
	Subsystem->StopAllFeel();
	TestWorld.Step(0.016f);

	// Works after feel.Enabled was set from the console (a higher priority than code).
	IConsoleVariable* Enabled = IConsoleManager::Get().FindConsoleVariable(TEXT("feel.Enabled"));
	Enabled->Set(true, ECVF_SetByConsole);
	UFeelBlueprintLibrary::SetFeelEnabled(false);
	TestFalse(TEXT("Set Feel Enabled works after a console change"), UFeelBlueprintLibrary::IsFeelEnabled());
	UFeelBlueprintLibrary::SetFeelEnabled(true);

	// Actor: starts in its Start Enabled state, toggles, and puts the switch back when the level ends.
	AFeelSwitch* Switch = TestWorld.World->SpawnActorDeferred<AFeelSwitch>(AFeelSwitch::StaticClass(), FTransform::Identity);
	Switch->bStartEnabled = false;
	Switch->FinishSpawning(FTransform::Identity);
	// This bare test world has no game mode, so start the actor's play the way a level does.
	Switch->DispatchBeginPlay();
	TestFalse(TEXT("Start Enabled = false turns FeelKit off when the level starts"), UFeelBlueprintLibrary::IsFeelEnabled());
	Switch->Switch();
	TestTrue(TEXT("Switch turns it on"), UFeelBlueprintLibrary::IsFeelEnabled());
	Switch->Switch();
	Switch->Tick(0.016f);
	TestFalse(TEXT("No display without a game viewport"), Switch->IsShowingDisplay());

	AddExpectedError(TEXT("already has a Feel Switch"), EAutomationExpectedErrorFlags::Contains, 1);
	AFeelSwitch* Second = TestWorld.World->SpawnActor<AFeelSwitch>();
	Second->DispatchBeginPlay();
	Second->Switch();
	TestTrue(TEXT("A second Feel Switch does not take over (its own Switch node still toggles)"), UFeelBlueprintLibrary::IsFeelEnabled());
	Second->Switch();
	Second->Destroy();
	TestFalse(TEXT("Removing the second switch leaves the state alone"), UFeelBlueprintLibrary::IsFeelEnabled());

	Switch->Destroy();
	TestTrue(TEXT("The level ending puts FeelKit back as it was found (on)"), UFeelBlueprintLibrary::IsFeelEnabled());

	// Text on screen names the keys.
	const AFeelSwitch* Defaults = GetDefault<AFeelSwitch>();
	TestEqual(TEXT("Default keys: Tab and the controller View / Share button"), Defaults->SwitchKeys.Num(), 2);
	TestEqual(TEXT("Start card hint"), Defaults->GetSwitchHint().ToString(), FString(TEXT("Press Tab (or View / Share on a controller) to turn the feel off and on.")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelSubsystemEnabledTest, "FeelKit.Runtime.EnabledConsoleVariable", FEEL_TEST_FLAGS)
bool FFeelSubsystemEnabledTest::RunTest(const FString& Parameters)
{
	using namespace FeelSubsystemTests;

	IConsoleVariable* Enabled = IConsoleManager::Get().FindConsoleVariable(TEXT("feel.Enabled"));
	FScopedTestWorld TestWorld;
	UFeelSubsystem* Subsystem = TestWorld.GetSubsystem();
	AWorldSettings* WorldSettings = TestWorld.World->GetWorldSettings();
	if (!TestNotNull(TEXT("feel.Enabled exists"), Enabled) || !TestNotNull(TEXT("Subsystem"), Subsystem) || !TestNotNull(TEXT("World settings"), WorldSettings))
	{
		return false;
	}

	WorldSettings->SetTimeDilation(1.0f);
	UFeelRecipe* Recipe = NewObject<UFeelRecipe>(GetTransientPackage());
	AddTrack(Recipe, NewObject<UFeelStep_GlobalHitstop>(Recipe), 0.0f, 1.0f);

	const FFeelHandle Handle = Subsystem->PlayFeel(Recipe, FFeelTarget());
	TestWorld.Step(0.1f);
	TestTrue(TEXT("Hitstop is active"), WorldSettings->TimeDilation < 1.0f);

	UFeelBlueprintLibrary::SetFeelEnabled(false);
	TestWorld.Step(0.1f);
	TestFalse(TEXT("feel.Enabled 0 stops playback"), Subsystem->IsPlaying(Handle));
	TestEqual(TEXT("feel.Enabled 0 restores time"), WorldSettings->TimeDilation, 1.0f);
	TestFalse(TEXT("No new plays while disabled"), Subsystem->PlayFeel(Recipe, FFeelTarget()).IsValid());

	UFeelBlueprintLibrary::SetFeelEnabled(true);
	TestTrue(TEXT("Plays again when re-enabled"), Subsystem->PlayFeel(Recipe, FFeelTarget()).IsValid());
	Subsystem->StopAllFeel();
	TestWorld.Step(0.016f);

	return true;
}

#undef FEEL_TEST_FLAGS

#endif // WITH_DEV_AUTOMATION_TESTS
