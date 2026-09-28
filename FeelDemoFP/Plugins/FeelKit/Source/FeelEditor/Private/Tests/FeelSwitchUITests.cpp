// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "FeelBlueprintLibrary.h"
#include "Components/TextBlock.h"
#include "FeelComfortMenu.h"
#include "FeelSwitch.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "ImageUtils.h"
#include "Misc/Paths.h"
#include "PlayInEditorDataTypes.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Tests/AutomationCommon.h"
#include "Widgets/SWindow.h"

/**
 * Plays in the editor with a Feel Switch in the level and switches it like a player: a real Tab key press and a real
 * controller View / Share button press through Slate, the way device input reaches the game. Checks the start card and
 * badge are shown, each press switches FeelKit, Tab does not move keyboard focus away from the game, O and the controller
 * Menu / Options button open the comfort menu (paused) and close it again, and leaving play puts FeelKit back on. Saves pictures of the display (Saved/FeelKit/FeelSwitch_<n>.png). Needs a rendering session.
 */
namespace FeelSwitchUITest
{
	struct FRun
	{
		int32 Stage = 0;
		double StageStart = 0.0;
		int32 Shots = 0;
		FString FocusBefore;
		TWeakObjectPtr<AFeelSwitch> Switch;
	};

	UWorld* PIEWorld()
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::PIE && Context.World())
			{
				return Context.World();
			}
		}
		return nullptr;
	}

	/** Pictures the whole play window, so Slate widgets over the game (the Feel Switch display) are included. */
	void Shot(FRun& Run, FAutomationTestBase* Test, const TCHAR* Label)
	{
		UWorld* World = PIEWorld();
		UGameViewportClient* Viewport = World ? World->GetGameViewport() : nullptr;
		TSharedPtr<SWindow> Window = Viewport ? Viewport->GetWindow() : nullptr;
		TArray<FColor> Pixels;
		FIntVector Size(0, 0, 0);
		if (!Window.IsValid() || !FSlateApplication::Get().TakeScreenshot(Window.ToSharedRef(), Pixels, Size) || Pixels.Num() == 0)
		{
			Test->AddInfo(TEXT("Feel Switch picture: the play window could not be read"));
			return;
		}
		for (FColor& Pixel : Pixels)
		{
			Pixel.A = 255;
		}
		const FString File = FPaths::ProjectSavedDir() / TEXT("FeelKit") / FString::Printf(TEXT("FeelSwitch_%d_%s.png"), Run.Shots++, Label);
		FImageUtils::SaveImageByExtension(*File, FImageView(Pixels.GetData(), Size.X, Size.Y, ERawImageFormat::BGRA8));
		Test->AddInfo(FString::Printf(TEXT("Feel Switch picture %s"), *File));
	}

	/** Presses and releases a key or controller button as device input does: through Slate to the focused game viewport. */
	void Press(const FKey& Key, bool bDown, bool bFocusGame = true)
	{
		FSlateApplication& Slate = FSlateApplication::Get();
		if (bFocusGame)
		{
			Slate.SetAllUserFocusToGameViewport();
		}
		const FKeyEvent Event(Key, FModifierKeysState(), 0, false, 0, 0);
		if (bDown)
		{
			Slate.ProcessKeyDownEvent(Event);
		}
		else
		{
			Slate.ProcessKeyUpEvent(Event);
		}
	}

	FString FocusedWidgetType()
	{
		const TSharedPtr<SWidget> Focused = FSlateApplication::Get().GetUserFocusedWidget(0);
		return Focused.IsValid() ? Focused->GetTypeAsString() : FString(TEXT("nothing"));
	}
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FFeelSwitchUICommand, TSharedRef<FeelSwitchUITest::FRun>, Run, FAutomationTestBase*, Test);

bool FFeelSwitchUICommand::Update()
{
	using namespace FeelSwitchUITest;
	const double Now = FPlatformTime::Seconds();
	if (Run->StageStart == 0.0)
	{
		Run->StageStart = Now;
	}
	const double Elapsed = Now - Run->StageStart;
	auto Next = [this, Now]()
	{
		++Run->Stage;
		Run->StageStart = Now;
	};
	UWorld* World = PIEWorld();
	AFeelSwitch* Switch = Run->Switch.Get();

	switch (Run->Stage)
	{
	case 0:
	{
		APlayerController* Controller = World ? World->GetFirstPlayerController() : nullptr;
		if (!Controller || !Controller->GetPawn() || Elapsed < 2.0)
		{
			if (Elapsed > 90.0)
			{
				Test->AddError(TEXT("Play In Editor did not start"));
				return true;
			}
			return false;
		}
		// Demo levels have their own Feel Switch; take it out so this test's switch is the one in charge.
		for (TActorIterator<AFeelSwitch> It(World); It; ++It)
		{
			It->Destroy();
		}
		AFeelSwitch* Spawned = World->SpawnActorDeferred<AFeelSwitch>(AFeelSwitch::StaticClass(), FTransform::Identity);
		// The longest title and text the demos use, so the picture shows whether they fit.
		Spawned->StartCardTitle = NSLOCTEXT("FeelSwitchTest", "Title", "FeelKit Demo: Every Bullet Has an Opinion");
		Spawned->StartCardText = NSLOCTEXT("FeelSwitchTest", "Text", "Unreal's Platforming template. Jump, wall jump, dash and drop off something high, then try it all again with the feel off.");
		// The longest controls list the demos use.
		Spawned->Controls = {
			{ NSLOCTEXT("FeelSwitchTest", "Move", "Move"), NSLOCTEXT("FeelSwitchTest", "MoveKeys", "WASD / Left stick") },
			{ NSLOCTEXT("FeelSwitchTest", "Attack", "Attack"), NSLOCTEXT("FeelSwitchTest", "AttackKeys", "Left mouse / RB") },
			{ NSLOCTEXT("FeelSwitchTest", "Charged", "Charged attack"), NSLOCTEXT("FeelSwitchTest", "ChargedKeys", "Hold right mouse / RT") },
			{ NSLOCTEXT("FeelSwitchTest", "Block", "Block, parry"), NSLOCTEXT("FeelSwitchTest", "BlockKeys", "Hold Left Shift / LT") },
		};
		Spawned->FinishSpawning(FTransform::Identity);
		Run->Switch = Spawned;
		Next();
		return false;
	}

	case 1:
		// Start card and badge on screen.
		if (Elapsed < 1.0)
		{
			return false;
		}
		Test->TestTrue(TEXT("The display is on screen"), Switch && Switch->IsShowingDisplay());
		Test->TestTrue(TEXT("The start card is visible"), Switch && Switch->IsStartCardVisible());
		Test->TestEqual(TEXT("The controls panel shows every row and the comfort menu row"), Switch ? Switch->GetShownControlRows() : 0, 5);
		Test->TestTrue(TEXT("FeelKit starts on"), UFeelBlueprintLibrary::IsFeelEnabled());
		Shot(*Run, Test, TEXT("start_card"));
		FSlateApplication::Get().SetAllUserFocusToGameViewport();
		Run->FocusBefore = FocusedWidgetType();
		Press(EKeys::Tab, true);
		Next();
		return false;

	case 2:
		if (Elapsed < 0.05)
		{
			return false;
		}
		Press(EKeys::Tab, false);
		Test->TestFalse(TEXT("Tab turns FeelKit off"), UFeelBlueprintLibrary::IsFeelEnabled());
		Shot(*Run, Test, TEXT("switch_confirm"));
		Next();
		return false;

	case 3:
		if (Elapsed < 1.0)
		{
			return false;
		}
		Test->TestFalse(TEXT("Switching closes the start card"), Switch && Switch->IsStartCardVisible());
		Test->AddInfo(FString::Printf(TEXT("Keyboard focus before Tab: %s, after: %s"), *Run->FocusBefore, *FocusedWidgetType()));
		Test->TestEqual(TEXT("Tab leaves keyboard focus on the game"), FocusedWidgetType(), Run->FocusBefore);
		Shot(*Run, Test, TEXT("off"));
		Press(EKeys::Gamepad_Special_Left, true);
		Next();
		return false;

	case 4:
		if (Elapsed < 0.05)
		{
			return false;
		}
		Press(EKeys::Gamepad_Special_Left, false);
		Test->TestTrue(TEXT("The controller View / Share button turns FeelKit on"), UFeelBlueprintLibrary::IsFeelEnabled());
		Next();
		return false;

	case 5:
		if (Elapsed < 0.8)
		{
			return false;
		}
		Shot(*Run, Test, TEXT("on"));
		// A second Tab still works (focus stayed on the game).
		Press(EKeys::Tab, true);
		Next();
		return false;

	case 6:
		if (Elapsed < 0.05)
		{
			return false;
		}
		Press(EKeys::Tab, false);
		Test->TestFalse(TEXT("A second Tab switches again"), UFeelBlueprintLibrary::IsFeelEnabled());
		Press(EKeys::O, true);
		Next();
		return false;

	case 7:
		if (Elapsed < 0.05)
		{
			return false;
		}
		Press(EKeys::O, false, false);
		Next();
		return false;

	case 8:
	{
		if (Elapsed < 0.8)
		{
			return false;
		}
		APlayerController* Controller = World ? World->GetFirstPlayerController() : nullptr;
		Test->TestNotNull(TEXT("O opens the comfort menu"), Switch ? Switch->GetOpenComfortMenu(Controller) : nullptr);
		Test->TestTrue(TEXT("The game is paused while the comfort menu is open"), UGameplayStatics::IsGamePaused(World));
		if (UFeelComfortMenu* Menu = Switch ? Switch->GetOpenComfortMenu(Controller) : nullptr)
		{
			const UTextBlock* Hint = Cast<UTextBlock>(Menu->GetWidgetFromName(TEXT("CloseHintText")));
			const FString HintString = Hint ? Hint->GetText().ToString() : FString();
			Test->AddInfo(FString::Printf(TEXT("Close hint: %s"), *HintString));
			Test->TestEqual(TEXT("The close hint names the Feel Switch key"), HintString, FString(TEXT("Esc or O to close, B / Circle on a controller")));
		}
		Shot(*Run, Test, TEXT("comfort_menu"));
		// The menu has keyboard focus now, so the key goes to it, as a player's key press would.
		Press(EKeys::O, true, false);
		Next();
		return false;
	}

	case 9:
	{
		if (Elapsed < 0.05)
		{
			return false;
		}
		Press(EKeys::O, false, false);
		APlayerController* Controller = World ? World->GetFirstPlayerController() : nullptr;
		Test->TestNull(TEXT("O closes the comfort menu"), Switch ? Switch->GetOpenComfortMenu(Controller) : nullptr);
		Test->TestFalse(TEXT("Closing the comfort menu unpauses the game"), UGameplayStatics::IsGamePaused(World));
		Next();
		return false;
	}

	case 10:
		if (Elapsed < 0.5)
		{
			return false;
		}
		Press(EKeys::Gamepad_Special_Right, true);
		Next();
		return false;

	case 11:
	{
		if (Elapsed < 0.05)
		{
			return false;
		}
		Press(EKeys::Gamepad_Special_Right, false, false);
		APlayerController* Controller = World ? World->GetFirstPlayerController() : nullptr;
		Test->TestNotNull(TEXT("The controller Menu / Options button opens the comfort menu"), Switch ? Switch->GetOpenComfortMenu(Controller) : nullptr);
		Next();
		return false;
	}

	case 12:
		if (Elapsed < 0.5)
		{
			return false;
		}
		Press(EKeys::Gamepad_Special_Right, true, false);
		Next();
		return false;

	case 13:
	{
		if (Elapsed < 0.05)
		{
			return false;
		}
		Press(EKeys::Gamepad_Special_Right, false, false);
		APlayerController* Controller = World ? World->GetFirstPlayerController() : nullptr;
		Test->TestNull(TEXT("The controller Menu / Options button closes the comfort menu"), Switch ? Switch->GetOpenComfortMenu(Controller) : nullptr);
		Test->TestFalse(TEXT("The game runs again"), UGameplayStatics::IsGamePaused(World));
		Test->TestFalse(TEXT("Closing the comfort menu leaves the feel as it was"), UFeelBlueprintLibrary::IsFeelEnabled());
		GEditor->RequestEndPlayMap();
		Next();
		return false;
	}

	default:
		if (Elapsed < 2.0)
		{
			return false;
		}
		Test->TestTrue(TEXT("Leaving play puts FeelKit back on"), UFeelBlueprintLibrary::IsFeelEnabled());
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelSwitchUITest, "FeelKit.Editor.FeelSwitchInPlay", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FFeelSwitchUITest::RunTest(const FString& Parameters)
{
	if (!FApp::CanEverRender() || !FSlateApplication::IsInitialized())
	{
		AddInfo(TEXT("Skipped: this session cannot render, so the game viewport and its widgets do not exist."));
		return true;
	}

	ULevelEditorPlaySettings* PlaySettings = NewObject<ULevelEditorPlaySettings>(GetTransientPackage());
	PlaySettings->SetPlayNumberOfClients(1);
	PlaySettings->SetPlayNetMode(EPlayNetMode::PIE_Standalone);
	PlaySettings->LastExecutedPlayModeType = EPlayModeType::PlayMode_InEditorFloating;

	FRequestPlaySessionParams Params;
	Params.EditorPlaySettings = PlaySettings;
	GEditor->RequestPlaySession(Params);
	ADD_LATENT_AUTOMATION_COMMAND(FFeelSwitchUICommand(MakeShared<FeelSwitchUITest::FRun>(), this));
	return true;
}

#endif
