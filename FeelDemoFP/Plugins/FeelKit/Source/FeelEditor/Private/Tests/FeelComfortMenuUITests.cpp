// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "FeelBlueprintLibrary.h"
#include "FeelComfortMenu.h"
#include "FeelComfortSubsystem.h"
#include "FeelSubsystem.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerController.h"
#include "ImageUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "PlayInEditorDataTypes.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Tests/AutomationCommon.h"
#include "Widgets/SWindow.h"

/**
 * Opens the comfort menu that ships with FeelKit in Play In Editor and uses it like a player: arrow keys to move and to
 * turn a slider down, a preset button, the Try button, the menu in a paused game, and the controller's B button to
 * close it. Checks each change reaches the player's comfort settings, that changes are saved, that previews play while
 * the game is paused, and that closing gives input and pause back. Saves pictures (Saved/FeelKit/ComfortMenu_<n>.png).
 * Restores the player's own comfort settings at the end. Needs a rendering session.
 */
namespace FeelComfortMenuUITest
{
	struct FRun
	{
		int32 Stage = 0;
		double StageStart = 0.0;
		int32 Shots = 0;
		int32 Presses = 0;
		FFeelComfortScales Original;
		TWeakObjectPtr<UFeelComfortMenu> Menu;
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

	void Shot(FRun& Run, FAutomationTestBase* Test, const TCHAR* Label)
	{
		UWorld* World = PIEWorld();
		UGameViewportClient* Viewport = World ? World->GetGameViewport() : nullptr;
		TSharedPtr<SWindow> Window = Viewport ? Viewport->GetWindow() : nullptr;
		TArray<FColor> Pixels;
		FIntVector Size(0, 0, 0);
		if (!Window.IsValid() || !FSlateApplication::Get().TakeScreenshot(Window.ToSharedRef(), Pixels, Size) || Pixels.Num() == 0)
		{
			Test->AddInfo(TEXT("Comfort menu picture: the play window could not be read"));
			return;
		}
		for (FColor& Pixel : Pixels)
		{
			Pixel.A = 255;
		}
		const FString File = FPaths::ProjectSavedDir() / TEXT("FeelKit") / FString::Printf(TEXT("ComfortMenu_%d_%s.png"), Run.Shots++, Label);
		FImageUtils::SaveImageByExtension(*File, FImageView(Pixels.GetData(), Size.X, Size.Y, ERawImageFormat::BGRA8));
		Test->AddInfo(FString::Printf(TEXT("Comfort menu picture %s"), *File));
	}

	/** A key press and release as a keyboard or controller sends it: through Slate to whatever has focus. */
	void Tap(const FKey& Key)
	{
		FSlateApplication& Slate = FSlateApplication::Get();
		const FKeyEvent Event(Key, FModifierKeysState(), 0, false, 0, 0);
		Slate.ProcessKeyDownEvent(Event);
		Slate.ProcessKeyUpEvent(Event);
	}

	UFeelComfortSubsystem* Comfort(UWorld* World)
	{
		return World ? UFeelBlueprintLibrary::GetFeelComfort(World->GetFirstPlayerController()) : nullptr;
	}

	template <typename T>
	T* Find(UFeelComfortMenu* Menu, const TCHAR* Name)
	{
		return Menu && Menu->WidgetTree ? Cast<T>(Menu->WidgetTree->FindWidget(FName(Name))) : nullptr;
	}

	FString TextOf(UFeelComfortMenu* Menu, const TCHAR* Name)
	{
		const UTextBlock* Block = Find<UTextBlock>(Menu, Name);
		return Block ? Block->GetText().ToString() : FString(TEXT("(missing)"));
	}
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FFeelComfortMenuUICommand, TSharedRef<FeelComfortMenuUITest::FRun>, Run, FAutomationTestBase*, Test);

bool FFeelComfortMenuUICommand::Update()
{
	using namespace FeelComfortMenuUITest;
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
	APlayerController* Controller = World ? World->GetFirstPlayerController() : nullptr;
	UFeelComfortSubsystem* Settings = Comfort(World);
	UFeelComfortMenu* Menu = Run->Menu.Get();
	UFeelSubsystem* Feel = World ? World->GetSubsystem<UFeelSubsystem>() : nullptr;

	switch (Run->Stage)
	{
	case 0:
		if (!Controller || !Controller->GetPawn() || !Settings || Elapsed < 2.0)
		{
			if (Elapsed > 90.0)
			{
				Test->AddError(TEXT("Play In Editor did not start"));
				return true;
			}
			return false;
		}
		Run->Original = Settings->GetComfortScales();
		Settings->ApplyComfortPreset(EFeelBuiltInComfortPreset::Default);
		Run->Menu = UFeelComfortMenu::ShowFeelComfortMenu(Controller, nullptr);
		Test->TestNotNull(TEXT("Show Feel Comfort Menu opens the menu from the project setting"), Run->Menu.Get());
		Next();
		return false;

	case 1:
		if (Elapsed < 1.0)
		{
			return false;
		}
		if (!Menu)
		{
			return true;
		}
		for (const TCHAR* Name : { TEXT("MasterSlider"), TEXT("CameraShakeSlider"), TEXT("CameraMotionSlider"), TEXT("FlashesSlider"), TEXT("HitstopSlider"),
			TEXT("ScreenDistortionSlider"), TEXT("HapticsSlider"), TEXT("ZoomSpeedSlider"), TEXT("CameraRollCheckBox"), TEXT("FlashLimiterCheckBox"),
			TEXT("DefaultPresetButton"), TEXT("ReducedMotionPresetButton"), TEXT("ReducedFlashingPresetButton"), TEXT("NoHapticsPresetButton"),
			TEXT("TryButton"), TEXT("ResetButton"), TEXT("CloseButton"), TEXT("DescriptionText"), TEXT("SaveStatusText") })
		{
			Test->TestNotNull(FString::Printf(TEXT("The menu has %s"), Name), Menu->WidgetTree ? Menu->WidgetTree->FindWidget(FName(Name)) : nullptr);
		}
		Test->TestTrue(TEXT("The mouse cursor shows while the menu is open"), Controller && Controller->bShowMouseCursor);
		Test->TestEqual(TEXT("Master shows 100%"), TextOf(Menu, TEXT("MasterValue")), FString(TEXT("100%")));
		Test->TestEqual(TEXT("The zoom speed shows no limit"), TextOf(Menu, TEXT("ZoomSpeedValue")), FString(TEXT("No limit")));
		Shot(*Run, Test, TEXT("open"));
		// Down once: from Master to Camera shake.
		Tap(EKeys::Down);
		Next();
		return false;

	case 2:
		// Four presses of Left turn Camera shake down by four steps of 5%.
		if (Elapsed < 0.15 * (Run->Presses + 1))
		{
			return false;
		}
		if (Run->Presses < 4)
		{
			Tap(EKeys::Left);
			++Run->Presses;
			return false;
		}
		Test->TestEqual(TEXT("Arrow keys turn Camera shake down to 80%"), Settings ? Settings->GetComfortScales().CameraShake : -1.0f, 0.8f, 0.001f);
		Test->TestEqual(TEXT("The value text follows"), TextOf(Menu, TEXT("CameraShakeValue")), FString(TEXT("80%")));
		Test->TestEqual(TEXT("The description explains the selected row"), TextOf(Menu, TEXT("DescriptionText")),
			FString(TEXT("How much the camera shakes, for example on hits and explosions.")));
		Shot(*Run, Test, TEXT("keyboard"));
		Next();
		return false;

	case 3:
		// Saved shortly after the last change.
		if (Elapsed < 1.2)
		{
			return false;
		}
		Test->TestEqual(TEXT("The change is saved"), TextOf(Menu, TEXT("SaveStatusText")), FString(TEXT("Saved")));
		if (UButton* Preset = Find<UButton>(Menu, TEXT("ReducedMotionPresetButton")))
		{
			Preset->OnClicked.Broadcast();
		}
		Test->TestEqual(TEXT("Reduced motion sets Camera shake to 25%"), Settings ? Settings->GetComfortScales().CameraShake : -1.0f, 0.25f, 0.001f);
		Test->TestEqual(TEXT("and the slider follows"), TextOf(Menu, TEXT("CameraShakeValue")), FString(TEXT("25%")));
		Test->TestFalse(TEXT("and turns camera roll off"), Settings && Settings->GetComfortScales().bAllowCameraRoll);
		Shot(*Run, Test, TEXT("preset"));
		if (UButton* Try = Find<UButton>(Menu, TEXT("TryButton")))
		{
			Try->OnClicked.Broadcast();
		}
		Test->TestTrue(TEXT("Try plays the sample"), Feel && Feel->GetNumActiveInstances() > 0);
		Next();
		return false;

	case 4:
		if (Elapsed < 0.06)
		{
			return false;
		}
		Shot(*Run, Test, TEXT("try"));
		if (Menu)
		{
			Menu->CloseMenu();
		}
		Test->TestFalse(TEXT("Closing hides the mouse cursor again"), Controller && Controller->bShowMouseCursor);
		// Open again, this time pausing the game.
		Run->Menu = UFeelComfortMenu::ShowFeelComfortMenu(Controller, nullptr, true);
		Test->TestTrue(TEXT("The menu can pause the game"), UGameplayStatics::IsGamePaused(World));
		if (UFeelComfortMenu* Paused = Run->Menu.Get())
		{
			Paused->PlayTry();
		}
		Next();
		return false;

	case 5:
		if (Elapsed < 1.5)
		{
			return false;
		}
		// The Try sample lasts half a second; it only finishes if FeelKit keeps running while the game is paused.
		Test->TestEqual(TEXT("Previews play and finish while the game is paused"), Feel ? Feel->GetNumActiveInstances() : -1, 0);
		Shot(*Run, Test, TEXT("paused"));
		// The controller's B button closes the menu.
		FSlateApplication::Get().SetAllUserFocus(Menu ? Menu->TakeWidget() : TSharedPtr<SWidget>(), EFocusCause::SetDirectly);
		Tap(EKeys::Gamepad_FaceButton_Right);
		Next();
		return false;

	case 6:
		if (Elapsed < 0.5)
		{
			return false;
		}
		Test->TestFalse(TEXT("B closes the menu"), Menu && Menu->IsInViewport());
		Test->TestFalse(TEXT("Closing unpauses the game"), UGameplayStatics::IsGamePaused(World));
		if (Settings)
		{
			Settings->SetComfortScales(Run->Original);
		}
		GEditor->RequestEndPlayMap();
		Next();
		return false;

	default:
		return Elapsed >= 2.0;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelComfortMenuUITest, "FeelKit.Editor.ComfortMenuInPlay", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FFeelComfortMenuUITest::RunTest(const FString& Parameters)
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
	ADD_LATENT_AUTOMATION_COMMAND(FFeelComfortMenuUICommand(MakeShared<FeelComfortMenuUITest::FRun>(), this));
	return true;
}

#endif
