// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Editor.h"
#include "FeelManualCapture.h"
#include "FeelRecipe.h"
#include "FeelRecipeEditorState.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "HAL/PlatformMisc.h"
#include "Misc/App.h"
#include "Misc/Paths.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Tests/AutomationCommon.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/SWindow.h"

/**
 * Diagnostic (filter DiagFeel): pictures of the recipe editor for the manual. Opens project recipes (the /Game copies
 * made for the manual), sizes the window, sets the application scale (default 1.25, environment variable
 * FEELKIT_SHOT_SCALE), opens the Intensity tab, sets the playhead and saves each window at twice its pixels
 * (FeelManualCapture) to Saved/FeelKit/Manual/<name>.png. The application scale is put back afterwards. Needs a
 * rendering session.
 */
namespace FeelManualShots
{
	struct FShot
	{
		const TCHAR* Name;
		const TCHAR* RecipePath;
		float Time;
		int32 SelectedTrack;
		bool bIntensityTab;
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelManualShotsDiagnostic, "DiagFeel.ManualShots", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FFeelManualShotsDiagnostic::RunTest(const FString& Parameters)
{
	using namespace FeelManualShots;
	if (!FApp::CanEverRender() || !FSlateApplication::IsInitialized() || !GEditor)
	{
		AddError(TEXT("Needs a rendering session."));
		return false;
	}

	const FString ScaleText = FPlatformMisc::GetEnvironmentVariable(TEXT("FEELKIT_SHOT_SCALE"));
	const float Scale = ScaleText.IsEmpty() ? 1.25f : FCString::Atof(*ScaleText);
	const TSharedRef<float> PreviousScale = MakeShared<float>(FSlateApplication::Get().GetApplicationScale());
	FSlateApplication::Get().SetApplicationScale(Scale);

	static const FShot Shots[] = {
		{ TEXT("Editor_HeavyHit"), TEXT("/Game/HeavyHit.HeavyHit"), 0.05f, INDEX_NONE, false },
		// At rest: at 0.05 s the scale punch squashes the preview mesh, which looks odd in a picture about the window.
		{ TEXT("Editor_HeavyHitRest"), TEXT("/Game/HeavyHit.HeavyHit"), 0.0f, INDEX_NONE, false },
		{ TEXT("Editor_ScalableHit"), TEXT("/Game/ScalableHit.ScalableHit"), 0.0f, INDEX_NONE, true },
	};

	// Each shot: open and arrange the editor, let the engine run for a few seconds (the preview's textures, lighting and
	// exposure need real engine frames), then take the picture and close the editor.
	for (const FShot& Shot : Shots)
	{
		const FShot* ShotPtr = &Shot;
		ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, ShotPtr]()
		{
			UFeelRecipe* Recipe = LoadObject<UFeelRecipe>(nullptr, ShotPtr->RecipePath);
			if (!Recipe)
			{
				AddWarning(FString::Printf(TEXT("MANUALSHOT %s: %s not found"), ShotPtr->Name, ShotPtr->RecipePath));
				return true;
			}
			UAssetEditorSubsystem* AssetEditors = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
			AssetEditors->CloseAllEditorsForAsset(Recipe);
			AssetEditors->OpenEditorForAsset(Recipe);
			IAssetEditorInstance* Editor = AssetEditors->FindEditorForAsset(Recipe, false);
			const TSharedPtr<FTabManager> EditorTabs = Editor ? Editor->GetAssociatedTabManager() : nullptr;
			const TSharedPtr<SDockTab> OwnerTab = EditorTabs.IsValid() ? EditorTabs->GetOwnerTab() : nullptr;
			const TSharedPtr<SWindow> Window = OwnerTab.IsValid() ? OwnerTab->GetParentWindow() : nullptr;
			if (!Window.IsValid())
			{
				return true;
			}
			const FName IntensityTab(TEXT("FeelRecipeEditor_Intensity"));
			if (ShotPtr->bIntensityTab)
			{
				EditorTabs->TryInvokeTab(IntensityTab);
			}
			else if (const TSharedPtr<SDockTab> Open = EditorTabs->FindExistingLiveTab(IntensityTab))
			{
				Open->RequestCloseTab();
			}
			Window->Resize(FVector2D(1900.0, 1060.0));
			if (const TSharedPtr<FFeelRecipeEditorState> State = FFeelRecipeEditorState::FindOpenState(Recipe))
			{
				State->SetTime(ShotPtr->Time);
				if (ShotPtr->SelectedTrack != INDEX_NONE)
				{
					State->SetSelectedTrack(ShotPtr->SelectedTrack);
				}
			}
			return true;
		}));
		ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(8.0f));
		ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, ShotPtr]()
		{
			UFeelRecipe* Recipe = LoadObject<UFeelRecipe>(nullptr, ShotPtr->RecipePath);
			UAssetEditorSubsystem* AssetEditors = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
			IAssetEditorInstance* Editor = Recipe ? AssetEditors->FindEditorForAsset(Recipe, false) : nullptr;
			const TSharedPtr<FTabManager> EditorTabs = Editor ? Editor->GetAssociatedTabManager() : nullptr;
			const TSharedPtr<SDockTab> OwnerTab = EditorTabs.IsValid() ? EditorTabs->GetOwnerTab() : nullptr;
			const TSharedPtr<SWindow> Window = OwnerTab.IsValid() ? OwnerTab->GetParentWindow() : nullptr;
			if (!FeelManualCapture::SaveWindow(Window, ShotPtr->Name, 2.0f, this))
			{
				return false;
			}
			if (Recipe)
			{
				AssetEditors->CloseAllEditorsForAsset(Recipe);
			}
			return true;
		}));
	}
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([PreviousScale]()
	{
		FSlateApplication::Get().SetApplicationScale(*PreviousScale);
		return true;
	}));
	return true;
}

#endif
