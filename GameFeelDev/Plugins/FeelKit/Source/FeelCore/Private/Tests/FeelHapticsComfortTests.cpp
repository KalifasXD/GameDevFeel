// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Misc/Paths.h"
#include "FeelComfortSubsystem.h"
#include "FeelSettings.h"
#include "GameFramework/PlayerController.h"
#include "Misc/ScopeExit.h"

#define FEEL_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

/**
 * Comfort scales the controller's vibration, and must never lose the value the game set itself. A player with Haptics
 * comfort 0 feels nothing at all, which is the one case that looks like a broken controller, so it is covered here.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelHapticsComfortScaleTest, "FeelKit.Comfort.EngineForceFeedbackScale", FEEL_TEST_FLAGS)
bool FFeelHapticsComfortScaleTest::RunTest(const FString& Parameters)
{
	// A minimal standalone game instance, which is enough to own a local player and its subsystems.
	UGameInstance* GameInstance = NewObject<UGameInstance>(GEngine);
	GameInstance->InitializeStandalone(TEXT("FeelHapticsComfortTestWorld"));
	UWorld* World = GameInstance->GetWorld();
	if (!TestNotNull(TEXT("The test world exists"), World))
	{
		return false;
	}

	// Built by hand instead of CreateLocalPlayer, which needs a game viewport this test does not have.
	ULocalPlayer* LocalPlayer = NewObject<ULocalPlayer>(GEngine, GEngine->LocalPlayerClass);
	LocalPlayer->PlayerAdded(nullptr, FPlatformUserId::CreateFromInternalId(0));
	APlayerController* PlayerController = World->SpawnActor<APlayerController>();
	ON_SCOPE_EXIT
	{
		LocalPlayer->PlayerRemoved();
		GameInstance->Shutdown();
	};

	if (!TestNotNull(TEXT("A local player exists"), LocalPlayer) || !TestNotNull(TEXT("A player controller exists"), PlayerController))
	{
		return false;
	}
	LocalPlayer->PlayerController = PlayerController;

	UFeelComfortSubsystem* Comfort = LocalPlayer->GetSubsystem<UFeelComfortSubsystem>();
	if (!TestNotNull(TEXT("The comfort subsystem exists"), Comfort))
	{
		return false;
	}

	UFeelSettings* Settings = GetMutableDefault<UFeelSettings>();
	const bool bPreviousApply = Settings->bApplyComfortToEngineForceFeedback;
	const bool bPreviousAutoSave = Settings->bAutoSaveComfort;
	Settings->bApplyComfortToEngineForceFeedback = true;
	Settings->bAutoSaveComfort = false;
	ON_SCOPE_EXIT
	{
		Settings->bApplyComfortToEngineForceFeedback = bPreviousApply;
		Settings->bAutoSaveComfort = bPreviousAutoSave;
	};

	// The game sets its own scale, for example from its own accessibility menu.
	PlayerController->ForceFeedbackScale = 0.8f;
	Comfort->SetComfortGroupScale(EFeelComfortGroup::Haptics, 1.0f);
	Comfort->SetMasterComfortScale(1.0f);
	TestEqual(TEXT("Full comfort keeps the game's own scale"), PlayerController->ForceFeedbackScale, 0.8f, 0.001f);
	TestEqual(TEXT("The effective comfort scale is 1"), Comfort->GetEffectiveForceFeedbackScale(), 1.0f, 0.001f);

	// Comfort multiplies that value instead of replacing it.
	Comfort->SetComfortGroupScale(EFeelComfortGroup::Haptics, 0.5f);
	TestEqual(TEXT("Half haptics comfort halves the game's scale"), PlayerController->ForceFeedbackScale, 0.4f, 0.001f);
	TestEqual(TEXT("The effective comfort scale is reported"), Comfort->GetEffectiveForceFeedbackScale(), 0.5f, 0.001f);

	Comfort->SetMasterComfortScale(0.5f);
	TestEqual(TEXT("Master comfort multiplies too"), PlayerController->ForceFeedbackScale, 0.2f, 0.001f);

	// Zero haptics silences the controller, and going back restores the game's value exactly.
	Comfort->SetComfortGroupScale(EFeelComfortGroup::Haptics, 0.0f);
	TestEqual(TEXT("Haptics comfort 0 stops all vibration"), PlayerController->ForceFeedbackScale, 0.0f, 0.001f);
	Comfort->SetComfortGroupScale(EFeelComfortGroup::Haptics, 1.0f);
	Comfort->SetMasterComfortScale(1.0f);
	TestEqual(TEXT("The game's scale comes back untouched"), PlayerController->ForceFeedbackScale, 0.8f, 0.001f);

	// Turning the setting off hands the controller back to the game.
	Comfort->SetComfortGroupScale(EFeelComfortGroup::Haptics, 0.25f);
	TestEqual(TEXT("Comfort applies again"), PlayerController->ForceFeedbackScale, 0.2f, 0.001f);
	Settings->bApplyComfortToEngineForceFeedback = false;
	Comfort->SetComfortGroupScale(EFeelComfortGroup::Haptics, 0.25f);
	TestEqual(TEXT("With the setting off the game's scale is restored"), PlayerController->ForceFeedbackScale, 0.8f, 0.001f);
	TestEqual(TEXT("And the effective scale is 1"), Comfort->GetEffectiveForceFeedbackScale(), 1.0f, 0.001f);

	// A scale the game sets while comfort is reduced is respected, not overwritten.
	Settings->bApplyComfortToEngineForceFeedback = true;
	Comfort->SetComfortGroupScale(EFeelComfortGroup::Haptics, 0.5f);
	PlayerController->ForceFeedbackScale = 0.6f; // the game changes its mind
	Comfort->SetComfortGroupScale(EFeelComfortGroup::Haptics, 1.0f);
	TestEqual(TEXT("The game's new scale is kept"), PlayerController->ForceFeedbackScale, 0.6f, 0.001f);

	return true;
}

#endif
