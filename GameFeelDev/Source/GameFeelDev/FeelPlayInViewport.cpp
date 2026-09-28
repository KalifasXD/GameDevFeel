// Tools (console commands, editor builds only):
// - DiagFeel.PlayInViewport starts Play In Editor in the active level viewport, the way the toolbar's Play button does
//   with "Selected Viewport".
// - DiagFeel.ShowDebugInViewport does the same, then, once the player has a pawn, enters "showdebug feel" and
//   "stat fps", prints a string and plays two sustained library recipes on the character, so the on-screen debug text
//   can be checked (and pictured from outside the editor) in that play mode.

#if WITH_EDITOR

#include "Containers/Ticker.h"
#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "FeelBlueprintLibrary.h"
#include "FeelRecipe.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "IAssetViewport.h"
#include "Kismet/KismetSystemLibrary.h"
#include "LevelEditor.h"
#include "Modules/ModuleManager.h"
#include "PlayInEditorDataTypes.h"
#include "Settings/LevelEditorPlaySettings.h"

namespace FeelPlayInViewport
{
	bool Start()
	{
		FLevelEditorModule& LevelEditor = FModuleManager::LoadModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
		TSharedPtr<IAssetViewport> Viewport = LevelEditor.GetFirstActiveViewport();
		if (!Viewport.IsValid() || !GEditor)
		{
			UE_LOG(LogTemp, Warning, TEXT("DiagFeel: no active level viewport"));
			return false;
		}
		ULevelEditorPlaySettings* PlaySettings = NewObject<ULevelEditorPlaySettings>(GetTransientPackage());
		PlaySettings->SetPlayNumberOfClients(1);
		PlaySettings->SetPlayNetMode(EPlayNetMode::PIE_Standalone);
		PlaySettings->LastExecutedPlayModeType = EPlayModeType::PlayMode_InViewPort;

		FRequestPlaySessionParams Params;
		Params.EditorPlaySettings = PlaySettings;
		Params.DestinationSlateViewport = Viewport;
		GEditor->RequestPlaySession(Params);
		return true;
	}

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
}

static FAutoConsoleCommand GFeelPlayInViewport(
	TEXT("DiagFeel.PlayInViewport"),
	TEXT("Starts Play In Editor in the active level viewport (Selected Viewport)."),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		FeelPlayInViewport::Start();
	}));

static FAutoConsoleCommand GFeelShowDebugInViewport(
	TEXT("DiagFeel.ShowDebugInViewport"),
	TEXT("Plays in the level viewport and turns on showdebug feel, stat fps, a printed string and two playing recipes."),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		if (!FeelPlayInViewport::Start())
		{
			return;
		}
		const double Started = FPlatformTime::Seconds();
		FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Started](float)
		{
			UWorld* World = FeelPlayInViewport::PIEWorld();
			APlayerController* Controller = World ? World->GetFirstPlayerController() : nullptr;
			if (!Controller || !Controller->GetPawn() || FPlatformTime::Seconds() - Started < 3.0)
			{
				return FPlatformTime::Seconds() - Started < 120.0;
			}
			Controller->ConsoleCommand(TEXT("showdebug feel"));
			Controller->ConsoleCommand(TEXT("stat fps"));
			UKismetSystemLibrary::PrintString(World, TEXT("Print String works"), true, true, FLinearColor(0.0f, 0.66f, 1.0f), 60.0f);
			const FFeelTarget Target = UFeelBlueprintLibrary::MakeFeelTargetFromActor(Controller->GetPawn());
			UFeelBlueprintLibrary::PlayFeel(World, LoadObject<UFeelRecipe>(nullptr, TEXT("/FeelKit/Library/Dread/FR_Dread_Heartbeat.FR_Dread_Heartbeat")), Target, 0.6f);
			UFeelBlueprintLibrary::PlayFeel(World, LoadObject<UFeelRecipe>(nullptr, TEXT("/FeelKit/Library/Danger/FR_Danger_LowHealth.FR_Danger_LowHealth")), Target, 0.4f);
			UE_LOG(LogTemp, Display, TEXT("DiagFeel.ShowDebugInViewport: commands entered"));
			return false;
		}));
	}));

#endif
