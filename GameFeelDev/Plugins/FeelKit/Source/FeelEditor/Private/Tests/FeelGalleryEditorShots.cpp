// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "AssetRegistry/AssetRegistryModule.h"
#include "ContentBrowserModule.h"
#include "Editor.h"
#include "FeelManualCapture.h"
#include "FeelRecipe.h"
#include "FeelRecipeEditorState.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "IContentBrowserSingleton.h"
#include "Misc/App.h"
#include "Misc/ConfigCacheIni.h"
#include "Modules/ModuleManager.h"
#include "Settings/ContentBrowserSettings.h"
#include "Steps/FeelStep_ScreenFlash.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "UObject/Package.h"
#include "Widgets/Docking/SDockTab.h"

/**
 * Diagnostic (filter DiagFeel): editor pictures for the store gallery, saved to Saved/FeelKit/Manual/Gallery_*.png at
 * twice the screen's pixels and 1.25x application scale: the recipe editor on a transient copy of FR_Impact_HeavyHit at two
 * moments of the hit, a transient copy of FR_Dread_JumpScare with its essential flash track selected and only the Comfort
 * category open, and the Content Browser showing every library recipe. Uses only what both editions contain, so the
 * same file takes the Lite pictures in a project with the Lite package (Tools/Run/lite_gallery_shots.ps1). Needs a
 * rendering session.
 */
namespace FeelGalleryEditorShots
{
	constexpr float Density = 2.0f;

	const TCHAR* RecipeCategories[] = { TEXT("FeelRecipe.Recipe"), TEXT("FeelRecipe.Parameters"), TEXT("FeelRecipe.Sustain"), TEXT("FeelRecipe.Library"),
		TEXT("FeelRecipe.Preview"), TEXT("FeelRecipe.Parameter Mappings"), TEXT("FeelRecipe.Randomness"), TEXT("FeelRecipe.Conditions"), TEXT("FeelRecipe.Comfort") };

	struct FRun
	{
		int32 Stage = 0;
		double StageStart = 0.0;
		float PreviousScale = 1.0f;
		bool bPreviousPluginFolders = false;
		TMap<FString, FString> SavedCategories;
		TWeakObjectPtr<UFeelRecipe> Recipe;
	};

	UFeelRecipe* TransientCopy(const TCHAR* LibraryPath, const TCHAR* Name)
	{
		UFeelRecipe* Library = LoadObject<UFeelRecipe>(nullptr, LibraryPath);
		if (!Library)
		{
			return nullptr;
		}
		UPackage* Package = CreatePackage(*FString::Printf(TEXT("/Temp/FeelKitGallery/%s"), Name));
		UFeelRecipe* Copy = DuplicateObject<UFeelRecipe>(Library, Package, Name);
		Copy->SetFlags(RF_Public | RF_Standalone | RF_Transactional);
		return Copy;
	}

	void OpenEditor(UFeelRecipe* Recipe)
	{
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(Recipe);
		if (const TSharedPtr<SWindow> Window = FeelManualCapture::AssetEditorWindow(Recipe))
		{
			Window->Resize(FVector2D(1900.0, 1060.0));
		}
		UAssetEditorSubsystem* AssetEditors = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
		IAssetEditorInstance* Editor = AssetEditors->FindEditorForAsset(Recipe, false);
		const TSharedPtr<FTabManager> Tabs = Editor ? Editor->GetAssociatedTabManager() : nullptr;
		if (const TSharedPtr<SDockTab> Intensity = Tabs.IsValid() ? Tabs->FindExistingLiveTab(FName(TEXT("FeelRecipeEditor_Intensity"))) : nullptr)
		{
			Intensity->RequestCloseTab();
		}
	}

	void CloseEditor(UFeelRecipe* Recipe)
	{
		if (Recipe)
		{
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->CloseAllEditorsForAsset(Recipe);
		}
	}
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FFeelGalleryEditorShotsCommand, TSharedRef<FeelGalleryEditorShots::FRun>, Run, FAutomationTestBase*, Test);

bool FFeelGalleryEditorShotsCommand::Update()
{
	using namespace FeelGalleryEditorShots;
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
	auto SetTime = [this](float Time)
	{
		if (const TSharedPtr<FFeelRecipeEditorState> State = FFeelRecipeEditorState::FindOpenState(Run->Recipe.Get()))
		{
			State->SetTime(Time);
		}
	};

	switch (Run->Stage)
	{
	case 0:
		Run->PreviousScale = FSlateApplication::Get().GetApplicationScale();
		FSlateApplication::Get().SetApplicationScale(1.25f);
		Run->Recipe = TransientCopy(TEXT("/FeelKit/Library/Impact/FR_Impact_HeavyHit.FR_Impact_HeavyHit"), TEXT("HeavyHit"));
		if (!Run->Recipe.IsValid())
		{
			Test->AddError(TEXT("FR_Impact_HeavyHit is missing."));
			return true;
		}
		OpenEditor(Run->Recipe.Get());
		Next();
		return false;

	case 1:
		if (Elapsed < 1.0)
		{
			return false;
		}
		SetTime(0.05f);
		Next();
		return false;

	case 2:
		// The preview needs real engine frames for its textures, lighting and exposure.
		if (Elapsed < 7.0 || !FeelManualCapture::SaveWindow(FeelManualCapture::AssetEditorWindow(Run->Recipe.Get()), TEXT("Gallery_Editor"), Density, Test))
		{
			return false;
		}
		SetTime(0.12f);
		// The Camera Punch track selected, so the Details panel shows a step's settings.
		if (const TSharedPtr<FFeelRecipeEditorState> State = FFeelRecipeEditorState::FindOpenState(Run->Recipe.Get()))
		{
			State->SetSelectedTrack(2);
		}
		Next();
		return false;

	case 3:
		if (Elapsed < 2.0 || !FeelManualCapture::SaveWindow(FeelManualCapture::AssetEditorWindow(Run->Recipe.Get()), TEXT("Gallery_Editor2"), Density, Test))
		{
			return false;
		}
		CloseEditor(Run->Recipe.Get());
		// Only the track and its Comfort category open, so the picture shows the essential settings.
		for (const TCHAR* Key : RecipeCategories)
		{
			FString Value;
			GConfig->GetString(TEXT("DetailCategories"), Key, Value, GEditorPerProjectIni);
			Run->SavedCategories.Add(Key, Value);
			GConfig->SetBool(TEXT("DetailCategories"), Key, FCString::Strcmp(Key, TEXT("FeelRecipe.Comfort")) == 0, GEditorPerProjectIni);
		}
		Run->Recipe = TransientCopy(TEXT("/FeelKit/Library/Dread/FR_Dread_JumpScare.FR_Dread_JumpScare"), TEXT("JumpScare"));
		if (!Run->Recipe.IsValid())
		{
			Test->AddError(TEXT("FR_Dread_JumpScare is missing."));
			return true;
		}
		OpenEditor(Run->Recipe.Get());
		Next();
		return false;

	case 4:
		if (Elapsed < 1.0)
		{
			return false;
		}
		if (const TSharedPtr<FFeelRecipeEditorState> State = FFeelRecipeEditorState::FindOpenState(Run->Recipe.Get()))
		{
			for (int32 Index = 0; Index < Run->Recipe->Tracks.Num(); ++Index)
			{
				if (Cast<UFeelStep_ScreenFlash>(Run->Recipe->Tracks[Index].Step))
				{
					State->SetSelectedTrack(Index);
					break;
				}
			}
		}
		Next();
		return false;

	case 5:
	{
		if (Elapsed < 5.0 || !FeelManualCapture::SaveWindow(FeelManualCapture::AssetEditorWindow(Run->Recipe.Get()), TEXT("Gallery_Essential"), Density, Test))
		{
			return false;
		}
		CloseEditor(Run->Recipe.Get());
		for (const TPair<FString, FString>& Pair : Run->SavedCategories)
		{
			if (Pair.Value.IsEmpty())
			{
				GConfig->RemoveKey(TEXT("DetailCategories"), *Pair.Key, GEditorPerProjectIni);
			}
			else
			{
				GConfig->SetString(TEXT("DetailCategories"), *Pair.Key, *Pair.Value, GEditorPerProjectIni);
			}
		}
		// The Content Browser showing every library recipe: with several folders selected it lists what they all hold.
		UContentBrowserSettings* Browser = GetMutableDefault<UContentBrowserSettings>();
		Run->bPreviousPluginFolders = Browser->GetDisplayPluginFolders();
		Browser->SetDisplayPluginFolders(true);
		Browser->PostEditChange();
		const TSharedPtr<SDockTab> Tab = FGlobalTabmanager::Get()->TryInvokeTab(FTabId(TEXT("ContentBrowserTab2")));
		if (const TSharedPtr<SWindow> Window = Tab.IsValid() ? Tab->GetParentWindow() : nullptr)
		{
			Window->Resize(FVector2D(1600.0, 900.0));
		}
		TArray<FAssetData> Recipes;
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get().GetAssetsByPath(FName(TEXT("/FeelKit/Library")), Recipes, true);
		TArray<FString> Folders;
		for (const FAssetData& Recipe : Recipes)
		{
			Folders.AddUnique(Recipe.PackagePath.ToString());
		}
		IContentBrowserSingleton& ContentBrowser = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser")).Get();
		ContentBrowser.SyncBrowserToFolders(Folders, false, true, TEXT("ContentBrowserTab2"));
		Next();
		return false;
	}

	case 6:
	{
		const TSharedPtr<SDockTab> Tab = FGlobalTabmanager::Get()->FindExistingLiveTab(FTabId(TEXT("ContentBrowserTab2")));
		if (Elapsed < 4.0 || !FeelManualCapture::SaveWindow(Tab.IsValid() ? Tab->GetParentWindow() : nullptr, TEXT("Gallery_ContentBrowser"), Density, Test))
		{
			return false;
		}
		if (Tab.IsValid())
		{
			Tab->RequestCloseTab();
		}
		UContentBrowserSettings* Browser = GetMutableDefault<UContentBrowserSettings>();
		Browser->SetDisplayPluginFolders(Run->bPreviousPluginFolders);
		Browser->PostEditChange();
		FSlateApplication::Get().SetApplicationScale(Run->PreviousScale);
		return true;
	}

	default:
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelGalleryEditorShotsDiagnostic, "DiagFeel.GalleryEditorShots", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FFeelGalleryEditorShotsDiagnostic::RunTest(const FString& Parameters)
{
	if (!FApp::CanEverRender() || !FSlateApplication::IsInitialized() || !GEditor)
	{
		AddError(TEXT("Needs a rendering session."));
		return false;
	}
	ADD_LATENT_AUTOMATION_COMMAND(FFeelGalleryEditorShotsCommand(MakeShared<FeelGalleryEditorShots::FRun>(), this));
	return true;
}

#endif
