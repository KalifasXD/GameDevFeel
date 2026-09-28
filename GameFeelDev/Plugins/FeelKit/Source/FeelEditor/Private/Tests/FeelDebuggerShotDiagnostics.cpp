// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "FeelBlueprintLibrary.h"
#include "FeelRecipe.h"
#include "FeelSubsystem.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "ImageUtils.h"
#include "Misc/Paths.h"
#include "PlayInEditorDataTypes.h"
#include "SFeelDebugger.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Tests/AutomationCommon.h"
#include "Widgets/Docking/SDockTab.h"

/**
 * Diagnostic (filter DiagFeel): starts Play In Editor, plays a few library recipes (one sustained, one that adds to the
 * Combo accumulator), opens the FeelKit Debugger and saves a picture of it to Saved/FeelKit/Debugger.png.
 */
namespace FeelDebuggerShotDiagnostics
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

	void Play(UWorld* World, APawn* Pawn, const TCHAR* Path, float Intensity)
	{
		if (UFeelRecipe* Recipe = LoadObject<UFeelRecipe>(nullptr, Path))
		{
			World->GetSubsystem<UFeelSubsystem>()->PlayFeel(Recipe, FFeelTarget::FromActor(Pawn), Intensity);
		}
	}
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FFeelDebuggerShotCommand, TSharedRef<FeelDebuggerShotDiagnostics::FRun>, Run, FAutomationTestBase*, Test);

bool FFeelDebuggerShotCommand::Update()
{
	using namespace FeelDebuggerShotDiagnostics;
	const double Now = FPlatformTime::Seconds();
	if (Run->StageStart == 0.0)
	{
		Run->StageStart = Now;
	}
	const double Elapsed = Now - Run->StageStart;
	UWorld* World = PIEWorld();
	APlayerController* Controller = World ? World->GetFirstPlayerController() : nullptr;
	APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;

	switch (Run->Stage)
	{
	case 0:
		if (Pawn && Elapsed > 3.0)
		{
			// Short plays that end, so Recent plays has entries.
			Play(World, Pawn, TEXT("/FeelKit/Library/Impact/FR_Impact_LightHit.FR_Impact_LightHit"), 1.0f);
			Play(World, Pawn, TEXT("/FeelKit/Library/Reward/FR_Reward_Pickup.FR_Reward_Pickup"), 0.8f);
			++Run->Stage;
			Run->StageStart = Now;
		}
		else if (Elapsed > 90.0)
		{
			Test->AddError(TEXT("DEBUGGERSHOT PIE did not start"));
			GEditor->RequestEndPlayMap();
			return true;
		}
		return false;

	case 1:
		if (Elapsed < 2.5)
		{
			return false;
		}
		// Plays that are still running when the picture is taken.
		UFeelBlueprintLibrary::AddToFeelAccumulator(World, TEXT("Combo"), 3.0f, nullptr);
		Play(World, Pawn, TEXT("/FeelKit/Library/Power/FR_Power_ChargeUp.FR_Power_ChargeUp"), 1.0f);
		Play(World, Pawn, TEXT("/FeelKit/Library/Danger/FR_Danger_LowHealth.FR_Danger_LowHealth"), 0.6f);
		FGlobalTabmanager::Get()->TryInvokeTab(SFeelDebugger::TabName);
		++Run->Stage;
		Run->StageStart = Now;
		return false;

	case 2:
	{
		if (Elapsed < 0.6)
		{
			return false;
		}
		const TSharedPtr<SDockTab> Tab = FGlobalTabmanager::Get()->FindExistingLiveTab(SFeelDebugger::TabName);
		TSharedPtr<SWindow> Window = Tab.IsValid() ? Tab->GetParentWindow() : nullptr;
		if (!Window.IsValid() && Tab.IsValid())
		{
			Window = FSlateApplication::Get().FindWidgetWindow(Tab->GetContent());
		}
		TArray<FColor> Pixels;
		FIntVector Size(0, 0, 0);
		if (Window.IsValid() && FSlateApplication::Get().TakeScreenshot(Window.ToSharedRef(), Pixels, Size) && Pixels.Num() > 0)
		{
			for (FColor& Pixel : Pixels)
			{
				Pixel.A = 255;
			}
			const FString File = FPaths::ProjectSavedDir() / TEXT("FeelKit") / TEXT("Debugger.png");
			FImageView Image(Pixels.GetData(), Size.X, Size.Y, ERawImageFormat::BGRA8);
			FImageUtils::SaveImageByExtension(*File, Image);
			Test->AddInfo(FString::Printf(TEXT("DEBUGGERSHOT wrote %s (%dx%d) from window %s"), *File, Size.X, Size.Y, *Window->GetTitle().ToString()));
		}
		else
		{
			Test->AddError(TEXT("DEBUGGERSHOT could not take the picture"));
		}
		GEditor->RequestEndPlayMap();
		++Run->Stage;
		Run->StageStart = Now;
		return false;
	}

	default:
		return Elapsed > 3.0;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelDebuggerShotDiagnostic, "DiagFeel.DebuggerShot", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FFeelDebuggerShotDiagnostic::RunTest(const FString& Parameters)
{
	ULevelEditorPlaySettings* PlaySettings = NewObject<ULevelEditorPlaySettings>(GetTransientPackage());
	PlaySettings->SetPlayNumberOfClients(1);
	PlaySettings->SetPlayNetMode(EPlayNetMode::PIE_Standalone);
	PlaySettings->LastExecutedPlayModeType = EPlayModeType::PlayMode_InViewPort;

	FRequestPlaySessionParams Params;
	Params.EditorPlaySettings = PlaySettings;
	GEditor->RequestPlaySession(Params);
	ADD_LATENT_AUTOMATION_COMMAND(FFeelDebuggerShotCommand(MakeShared<FeelDebuggerShotDiagnostics::FRun>(), this));
	return true;
}

#endif
