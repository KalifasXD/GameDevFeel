// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Editor.h"
#include "Engine/World.h"
#include "FeelComfortSubsystem.h"
#include "FeelBlueprintLibrary.h"
#include "FeelManualCapture.h"
#include "FeelPlayCapture.h"
#include "FeelRecipe.h"
#include "FeelRecipeEditorState.h"
#include "FeelSettings.h"
#include "FeelSubsystem.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "GameFramework/PlayerController.h"
#include "Misc/App.h"
#include "PlayInEditorDataTypes.h"
#include "SFeelDebugger.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "UObject/Package.h"
#include "Widgets/Docking/SDockTab.h"

/**
 * Diagnostic (filter DiagFeel): the pictures of the manual's debugging chapter, saved to Saved/FeelKit/Manual/Debug_*.png
 * at twice the screen's pixels and 1.25x application scale. A transient copy of the project's HeavyHit whose Camera Punch
 * track has a Max Distance of 100 cm plays during Play In Editor with the player's Camera Shake comfort at 0.25, next to
 * a sustained FR_Power_ChargeUp and FR_Impact_ScalableHit with Damage 80. Pictures: the FeelKit Debugger while they play,
 * then, after play has stopped, the copy's recipe editor with the Recent Plays menu open, and the same editor replaying the
 * recorded play, whose track labels show the distance condition and the comfort scale. Needs a rendering session and the
 * /Game/HeavyHit copy.
 */
namespace FeelDebuggingShots
{
	constexpr float Density = 2.0f;

	struct FRun
	{
		int32 Stage = 0;
		double StageStart = 0.0;
		float PreviousScale = 1.0f;
		TWeakObjectPtr<UFeelRecipe> Copy;
		FFeelHandle Charge;
		bool bClicked = false;
	};

	UFeelRecipe* Library(const TCHAR* Path)
	{
		return LoadObject<UFeelRecipe>(nullptr, Path);
	}

	TSharedPtr<SWindow> DebuggerWindow()
	{
		const TSharedPtr<SDockTab> Tab = FGlobalTabmanager::Get()->FindExistingLiveTab(SFeelDebugger::TabName);
		return Tab.IsValid() ? Tab->GetParentWindow() : nullptr;
	}
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FFeelDebuggingShotsCommand, TSharedRef<FeelDebuggingShots::FRun>, Run, FAutomationTestBase*, Test);

bool FFeelDebuggingShotsCommand::Update()
{
	using namespace FeelDebuggingShots;
	using FeelManualCapture::SaveWindow;
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
	UWorld* World = FeelManualCapture::PIEWorld();
	APlayerController* Controller = World ? World->GetFirstPlayerController() : nullptr;
	APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
	UFeelSubsystem* Subsystem = World ? World->GetSubsystem<UFeelSubsystem>() : nullptr;

	switch (Run->Stage)
	{
	case 0:
	{
		UFeelRecipe* Source = LoadObject<UFeelRecipe>(nullptr, TEXT("/Game/HeavyHit.HeavyHit"));
		if (!Source)
		{
			Test->AddError(TEXT("The project copy /Game/HeavyHit is missing."));
			return true;
		}
		UPackage* Package = CreatePackage(TEXT("/Temp/FeelKitManualDebugging/HeavyHit"));
		UFeelRecipe* Copy = DuplicateObject<UFeelRecipe>(Source, Package, TEXT("HeavyHit"));
		Copy->SetFlags(RF_Public | RF_Standalone | RF_Transactional);
		for (FFeelTrack& Track : Copy->Tracks)
		{
			if (Track.Step && Track.Step->GetClass()->GetName() == TEXT("FeelStep_CameraPunch"))
			{
				Track.Conditions.MaxDistance = 100.0f;
			}
		}
		Run->Copy = Copy;
		FFeelPlayCaptureStore::Get().Clear();

		ULevelEditorPlaySettings* PlaySettings = NewObject<ULevelEditorPlaySettings>(GetTransientPackage());
		PlaySettings->SetPlayNumberOfClients(1);
		PlaySettings->SetPlayNetMode(EPlayNetMode::PIE_Standalone);
		PlaySettings->LastExecutedPlayModeType = EPlayModeType::PlayMode_InEditorFloating;
		FRequestPlaySessionParams Params;
		Params.EditorPlaySettings = PlaySettings;
		GEditor->RequestPlaySession(Params);
		Next();
		return false;
	}

	case 1:
	{
		if (!Pawn || !Subsystem || Elapsed < 3.0)
		{
			return Elapsed > 90.0;
		}
		if (UFeelComfortSubsystem* Comfort = UFeelBlueprintLibrary::GetFeelComfort(Controller))
		{
			// Only for this play session: a player who turned camera shake down to a quarter.
			FFeelComfortScales Scales = GetDefault<UFeelSettings>()->DefaultComfortScales;
			Scales.CameraShake = 0.25f;
			Comfort->SetComfortScalesWithoutSaving(Scales);
		}
		const FFeelTarget Target = FFeelTarget::FromActor(Pawn);
		// Two plays that end before the picture, so Recent plays has entries.
		Subsystem->PlayFeel(Run->Copy.Get(), Target);
		Subsystem->PlayFeel(Library(TEXT("/FeelKit/Library/Reward/FR_Reward_Pickup.FR_Reward_Pickup")), Target, 0.8f);
		FGlobalTabmanager::Get()->TryInvokeTab(SFeelDebugger::TabName);
		Next();
		return false;
	}

	case 2:
	{
		if (Elapsed < 2.0 || !Subsystem || !Pawn)
		{
			return false;
		}
		// Plays that are still running when the picture is taken.
		const FFeelTarget Target = FFeelTarget::FromActor(Pawn);
		Subsystem->AddToAccumulator(TEXT("Combo"), 3.0f, Pawn);
		Run->Charge = Subsystem->PlayFeel(Library(TEXT("/FeelKit/Library/Power/FR_Power_ChargeUp.FR_Power_ChargeUp")), Target);
		FFeelPlayContext Context;
		Context.Parameters.Add(TEXT("Damage"), 80.0f);
		Subsystem->PlayFeel(Library(TEXT("/FeelKit/Library/Impact/FR_Impact_ScalableHit.FR_Impact_ScalableHit")), Target, 1.0f, Context);
		Subsystem->PlayFeel(Run->Copy.Get(), Target);
		Next();
		return false;
	}

	case 3:
		if (Elapsed < 0.15 || !SaveWindow(DebuggerWindow(), TEXT("Debug_Debugger"), Density, Test))
		{
			return false;
		}
		Next();
		return false;

	case 4:
		if (Elapsed < 1.0)
		{
			return false;
		}
		if (Subsystem)
		{
			Subsystem->ReleaseFeel(Run->Charge);
		}
		Next();
		return false;

	case 5:
		if (Elapsed < 2.5)
		{
			return false;
		}
		GEditor->RequestEndPlayMap();
		Next();
		return false;

	case 6:
	{
		if (Elapsed < 2.0)
		{
			return false;
		}
		if (const TSharedPtr<SDockTab> Tab = FGlobalTabmanager::Get()->FindExistingLiveTab(SFeelDebugger::TabName))
		{
			Tab->RequestCloseTab();
		}
		UFeelRecipe* Copy = Run->Copy.Get();
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(Copy);
		if (const TSharedPtr<SWindow> Window = FeelManualCapture::AssetEditorWindow(Copy))
		{
			Window->Resize(FVector2D(1900.0, 1060.0));
		}
		IAssetEditorInstance* Editor = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(Copy, false);
		const TSharedPtr<FTabManager> Tabs = Editor ? Editor->GetAssociatedTabManager() : nullptr;
		if (const TSharedPtr<SDockTab> Intensity = Tabs.IsValid() ? Tabs->FindExistingLiveTab(FName(TEXT("FeelRecipeEditor_Intensity"))) : nullptr)
		{
			Intensity->RequestCloseTab();
		}
		Next();
		return false;
	}

	case 7:
	{
		if (Elapsed < 3.0)
		{
			return false;
		}
		const TSharedPtr<SWindow> Window = FeelManualCapture::AssetEditorWindow(Run->Copy.Get());
		if (!Run->bClicked && Window.IsValid())
		{
			// One click: a second one would close the menu again.
			Run->bClicked = true;
			FeelManualCapture::Click(FeelManualCapture::FindText(Window.ToSharedRef(), TEXT("Recent Plays")));
			return false;
		}
		if (Elapsed < 4.5 || !SaveWindow(Window, TEXT("Debug_RecentMenu"), Density, Test))
		{
			return false;
		}
		FSlateApplication::Get().DismissAllMenus();
		Next();
		return false;
	}

	case 8:
	{
		// Replay the last recorded play of the copy, frozen where its tracks are all under way.
		const TSharedPtr<FFeelRecipeEditorState> State = FFeelRecipeEditorState::FindOpenState(Run->Copy.Get());
		const TArray<FFeelPlayCapture>& Captures = FFeelPlayCaptureStore::Get().GetCaptures();
		for (int32 Index = Captures.Num() - 1; State.IsValid() && Index >= 0; --Index)
		{
			if (Captures[Index].Recipe.Get() == Run->Copy.Get())
			{
				State->LoadCapture(Captures[Index]);
				break;
			}
		}
		Next();
		return false;
	}

	case 9:
	{
		if (Elapsed < 1.0)
		{
			return false;
		}
		const TSharedPtr<FFeelRecipeEditorState> State = FFeelRecipeEditorState::FindOpenState(Run->Copy.Get());
		if (State.IsValid() && Elapsed < 1.2)
		{
			if (State->IsPlaying())
			{
				State->TogglePlay();
			}
			State->SetTime(0.12f);
			return false;
		}
		if (Elapsed < 4.0 || !SaveWindow(FeelManualCapture::AssetEditorWindow(Run->Copy.Get()), TEXT("Debug_Replay"), Density, Test))
		{
			return false;
		}
		if (State.IsValid())
		{
			State->ClearCapture();
		}
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->CloseAllEditorsForAsset(Run->Copy.Get());
		Next();
		return false;
	}

	default:
		if (Elapsed < 1.0)
		{
			return false;
		}
		FSlateApplication::Get().SetApplicationScale(Run->PreviousScale);
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelDebuggingShotsDiagnostic, "DiagFeel.ManualShotsDebugging", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FFeelDebuggingShotsDiagnostic::RunTest(const FString& Parameters)
{
	if (!FApp::CanEverRender() || !FSlateApplication::IsInitialized() || !GEditor)
	{
		AddError(TEXT("Needs a rendering session."));
		return false;
	}
	const TSharedRef<FeelDebuggingShots::FRun> Run = MakeShared<FeelDebuggingShots::FRun>();
	Run->PreviousScale = FSlateApplication::Get().GetApplicationScale();
	FSlateApplication::Get().SetApplicationScale(1.25f);
	ADD_LATENT_AUTOMATION_COMMAND(FFeelDebuggingShotsCommand(Run, this));
	return true;
}

#endif
