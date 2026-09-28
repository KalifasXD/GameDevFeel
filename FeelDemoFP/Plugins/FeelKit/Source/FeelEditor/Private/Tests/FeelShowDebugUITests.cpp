// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "FeelBlueprintLibrary.h"
#include "FeelRecipe.h"
#include "FeelSubsystem.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/HUD.h"
#include "GameFramework/PlayerController.h"
#include "ImageUtils.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/Paths.h"
#include "PlayInEditorDataTypes.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Tests/AutomationCommon.h"
#include "Widgets/SWindow.h"

/**
 * Plays in the editor the way a user checks FeelKit's on-screen debug: enters "showdebug feel" in the console, prints a
 * string, plays two sustained library recipes on the character and sets an accumulator. Checks the HUD shows the Feel
 * page and on-screen messages are on, then saves a picture of the play window (Saved/FeelKit/ShowDebug_<label>.png).
 * Slate's window picture leaves out the debug text layer, so the text itself is not in that picture; with the
 * environment variable FEELKIT_HOLD_PIE set to seconds, play stays open that long so the window can be pictured from
 * outside the editor (checked 2026-09-24: the lines, Print String and stat fps all show). Needs a rendering session.
 */
namespace FeelShowDebugUITest
{
	struct FRun
	{
		int32 Stage = 0;
		double StageStart = 0.0;
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

	void Shot(FAutomationTestBase* Test, const TCHAR* Label)
	{
		UWorld* World = PIEWorld();
		UGameViewportClient* Viewport = World ? World->GetGameViewport() : nullptr;
		TSharedPtr<SWindow> Window = Viewport ? Viewport->GetWindow() : nullptr;
		TArray<FColor> Pixels;
		FIntVector Size(0, 0, 0);
		if (!Window.IsValid() || !FSlateApplication::Get().TakeScreenshot(Window.ToSharedRef(), Pixels, Size) || Pixels.Num() == 0)
		{
			Test->AddInfo(TEXT("Show debug picture: the play window could not be read"));
			return;
		}
		for (FColor& Pixel : Pixels)
		{
			Pixel.A = 255;
		}
		const FString File = FPaths::ProjectSavedDir() / TEXT("FeelKit") / FString::Printf(TEXT("ShowDebug_%s.png"), Label);
		FImageUtils::SaveImageByExtension(*File, FImageView(Pixels.GetData(), Size.X, Size.Y, ERawImageFormat::BGRA8));
		Test->AddInfo(FString::Printf(TEXT("Show debug picture %s"), *File));
	}
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FFeelShowDebugUICommand, TSharedRef<FeelShowDebugUITest::FRun>, Run, FAutomationTestBase*, Test);

bool FFeelShowDebugUICommand::Update()
{
	using namespace FeelShowDebugUITest;
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

	switch (Run->Stage)
	{
	case 0:
	{
		if (!Controller || !Controller->GetPawn() || Elapsed < 3.0)
		{
			if (Elapsed > 90.0)
			{
				Test->AddError(TEXT("Play In Editor did not start"));
				return true;
			}
			return false;
		}
		Controller->ConsoleCommand(TEXT("showdebug feel"));
		Controller->ConsoleCommand(TEXT("stat fps"));
		UKismetSystemLibrary::PrintString(World, TEXT("Print String works"), true, true, FLinearColor(0.0f, 0.66f, 1.0f), 30.0f);

		UFeelRecipe* Heartbeat = LoadObject<UFeelRecipe>(nullptr, TEXT("/FeelKit/Library/Dread/FR_Dread_Heartbeat.FR_Dread_Heartbeat"));
		UFeelRecipe* LowHealth = LoadObject<UFeelRecipe>(nullptr, TEXT("/FeelKit/Library/Danger/FR_Danger_LowHealth.FR_Danger_LowHealth"));
		Test->TestNotNull(TEXT("Heartbeat recipe loads"), Heartbeat);
		Test->TestNotNull(TEXT("Low Health recipe loads"), LowHealth);
		const FFeelTarget Target = UFeelBlueprintLibrary::MakeFeelTargetFromActor(Controller->GetPawn());
		UFeelBlueprintLibrary::PlayFeel(World, Heartbeat, Target, 0.6f);
		UFeelBlueprintLibrary::PlayFeel(World, LowHealth, Target, 0.4f);
		UFeelBlueprintLibrary::SetFeelAccumulator(World, TEXT("Combo"), 3.0f);
		Next();
		return false;
	}

	case 1:
	{
		if (Elapsed < 1.5)
		{
			return false;
		}
		const AHUD* HUD = Controller ? Controller->GetHUD() : nullptr;
		Test->AddInfo(FString::Printf(TEXT("HUD %s, show HUD %d, show debug info %d, Feel page %d, on-screen messages %d / %d"),
			HUD ? *HUD->GetClass()->GetName() : TEXT("none"),
			HUD ? HUD->bShowHUD : -1,
			HUD ? HUD->bShowDebugInfo : -1,
			HUD ? HUD->ShouldDisplayDebug(TEXT("Feel")) : -1,
			GEngine->bEnableOnScreenDebugMessages ? 1 : 0,
			GAreScreenMessagesEnabled ? 1 : 0));
		Test->TestNotNull(TEXT("The player has a HUD"), HUD);
		Test->TestTrue(TEXT("showdebug feel turns on the Feel page"), HUD && HUD->bShowDebugInfo && HUD->ShouldDisplayDebug(TEXT("Feel")));
		const UFeelSubsystem* Feel = World->GetSubsystem<UFeelSubsystem>();
		Test->TestTrue(TEXT("Two recipes are playing"), Feel && Feel->GetNumActiveInstances() >= 2);
		Shot(Test, TEXT("play"));
		Next();
		return false;
	}

	case 2:
		// Stays in play for a moment so the window can also be pictured from outside the editor.
		if (Elapsed < FCString::Atof(*FString(FPlatformMisc::GetEnvironmentVariable(TEXT("FEELKIT_HOLD_PIE"))).TrimStartAndEnd()))
		{
			return false;
		}
		GEditor->RequestEndPlayMap();
		Next();
		return false;

	default:
		return Elapsed > 2.0;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelShowDebugUITest, "FeelKit.Editor.ShowDebugInPlay", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FFeelShowDebugUITest::RunTest(const FString& Parameters)
{
	if (!FApp::CanEverRender() || !FSlateApplication::IsInitialized())
	{
		AddInfo(TEXT("Skipped: this session cannot render, so the game viewport and its HUD do not exist."));
		return true;
	}

	// With FEELKIT_PIE_STARTED set, a play session is started by other means (for example in the level viewport) and the
	// test uses it.
	if (!FPlatformMisc::GetEnvironmentVariable(TEXT("FEELKIT_PIE_STARTED")).IsEmpty())
	{
		ADD_LATENT_AUTOMATION_COMMAND(FFeelShowDebugUICommand(MakeShared<FeelShowDebugUITest::FRun>(), this));
		return true;
	}

	ULevelEditorPlaySettings* PlaySettings = NewObject<ULevelEditorPlaySettings>(GetTransientPackage());
	PlaySettings->SetPlayNumberOfClients(1);
	PlaySettings->SetPlayNetMode(EPlayNetMode::PIE_Standalone);
	PlaySettings->LastExecutedPlayModeType = EPlayModeType::PlayMode_InEditorFloating;

	FRequestPlaySessionParams Params;
	Params.EditorPlaySettings = PlaySettings;
	GEditor->RequestPlaySession(Params);
	ADD_LATENT_AUTOMATION_COMMAND(FFeelShowDebugUICommand(MakeShared<FeelShowDebugUITest::FRun>(), this));
	return true;
}

#endif
