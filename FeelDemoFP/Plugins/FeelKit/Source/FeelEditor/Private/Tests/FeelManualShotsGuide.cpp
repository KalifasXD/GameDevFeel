// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "AssetRegistry/AssetRegistryModule.h"
#include "ContentBrowserModule.h"
#include "Editor.h"
#include "FeelManualCapture.h"
#include "FeelRecipe.h"
#include "FeelRecipeEditorState.h"
#include "FileHelpers.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "IContentBrowserSingleton.h"
#include "Misc/App.h"
#include "Modules/ModuleManager.h"
#include "ObjectTools.h"
#include "Settings/ContentBrowserSettings.h"
#include "Steps/FeelStep_PlaySound.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "UObject/Package.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Text/STextBlock.h"

/**
 * Diagnostic (filter DiagFeel): pictures of the manual's guide chapters (installing, the recipe editor), saved to
 * Saved/FeelKit/Manual/Guide_*.png at twice the screen's pixels and 1.25x application scale: the Plugins window searched
 * for FeelKit, the Content Browser at FeelKit Content and its Settings menu, the HeavyHit copy with a track selected,
 * its Comfort menu, a transient copy of FR_Power_ChargeUp looping in its sustain region, and the Asset Check page after
 * saving a recipe that has a Play Sound track without a sound (saved to /Game/FeelKitManualTemp and deleted again).
 * Needs a rendering session and the /Game/HeavyHit copy.
 */
namespace FeelGuideShots
{
	constexpr float Density = 2.0f;

	struct FRun
	{
		int32 Stage = 0;
		double StageStart = 0.0;
		float PreviousScale = 1.0f;
		bool bPreviousPluginFolders = false;
		TWeakObjectPtr<UFeelRecipe> Sustained;
		TWeakObjectPtr<UFeelRecipe> Invalid;
	};

	TSharedPtr<SWindow> TabWindow(const TCHAR* TabId)
	{
		const TSharedPtr<SDockTab> Tab = FGlobalTabmanager::Get()->FindExistingLiveTab(FTabId(TabId));
		return Tab.IsValid() ? Tab->GetParentWindow() : nullptr;
	}

	void CloseTab(const TCHAR* TabId)
	{
		if (const TSharedPtr<SDockTab> Tab = FGlobalTabmanager::Get()->FindExistingLiveTab(FTabId(TabId)))
		{
			Tab->RequestCloseTab();
		}
	}

	/** Opens the combo button that holds the text (for example a toolbar menu). */
	bool OpenMenuWithText(const TSharedPtr<SWindow>& Window, const FString& Text, bool bPrefix)
	{
		const TSharedPtr<SWidget> Label = Window.IsValid() ? FeelManualCapture::FindText(Window.ToSharedRef(), Text, bPrefix) : nullptr;
		const TSharedPtr<SWidget> Combo = FeelManualCapture::FindAncestor(Label, TEXT("SComboButton"));
		if (Combo.IsValid())
		{
			StaticCastSharedPtr<SComboButton>(Combo)->SetIsOpen(true);
			return true;
		}
		return false;
	}

	void CollectWidgets(const TSharedRef<SWidget>& Root, const FName Type, TArray<TSharedRef<SWidget>>& Out)
	{
		if (Root->GetType() == Type)
		{
			Out.Add(Root);
		}
		FChildren* Children = Root->GetChildren();
		for (int32 Index = 0; Children && Index < Children->Num(); ++Index)
		{
			CollectWidgets(Children->GetChildAt(Index), Type, Out);
		}
	}

	/** Opens the Content Browser's settings menu: the gear, the right-most action button without a text label. */
	bool OpenContentBrowserSettings(const TSharedPtr<SWindow>& Window)
	{
		TArray<TSharedRef<SWidget>> Buttons;
		if (Window.IsValid())
		{
			CollectWidgets(Window.ToSharedRef(), TEXT("SActionButton"), Buttons);
		}
		TSharedPtr<SWidget> Best;
		float BestX = -1.0f;
		for (const TSharedRef<SWidget>& Button : Buttons)
		{
			TArray<TSharedRef<SWidget>> Texts;
			CollectWidgets(Button, TEXT("STextBlock"), Texts);
			const bool bHasLabel = Texts.ContainsByPredicate([](const TSharedRef<SWidget>& Text) { return !StaticCastSharedRef<STextBlock>(Text)->GetText().IsEmpty(); });
			const float X = Button->GetTickSpaceGeometry().GetAbsolutePosition().X;
			if (!bHasLabel && X > BestX)
			{
				Best = Button;
				BestX = X;
			}
		}
		const TSharedPtr<SWidget> Combo = Best.IsValid() ? FeelManualCapture::FindWidget(Best.ToSharedRef(), TEXT("SComboButton")) : nullptr;
		if (Combo.IsValid())
		{
			StaticCastSharedPtr<SComboButton>(Combo)->SetIsOpen(true);
			return true;
		}
		return false;
	}

	TSharedPtr<SWindow> OpenEditor(UObject* Asset)
	{
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(Asset);
		const TSharedPtr<SWindow> Window = FeelManualCapture::AssetEditorWindow(Asset);
		if (Window.IsValid())
		{
			Window->Resize(FVector2D(1900.0, 1060.0));
		}
		return Window;
	}

	void CloseEditor(UObject* Asset)
	{
		if (Asset)
		{
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->CloseAllEditorsForAsset(Asset);
		}
	}

	void CloseIntensityTab(UObject* Asset)
	{
		IAssetEditorInstance* Editor = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(Asset, false);
		const TSharedPtr<FTabManager> Tabs = Editor ? Editor->GetAssociatedTabManager() : nullptr;
		if (const TSharedPtr<SDockTab> Intensity = Tabs.IsValid() ? Tabs->FindExistingLiveTab(FTabId(TEXT("FeelRecipeEditor_Intensity"))) : nullptr)
		{
			Intensity->RequestCloseTab();
		}
	}
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FFeelGuideShotsCommand, TSharedRef<FeelGuideShots::FRun>, Run, FAutomationTestBase*, Test);

bool FFeelGuideShotsCommand::Update()
{
	using namespace FeelGuideShots;
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
	UFeelRecipe* HeavyHit = LoadObject<UFeelRecipe>(nullptr, TEXT("/Game/HeavyHit.HeavyHit"));

	switch (Run->Stage)
	{
	case 0:
		FGlobalTabmanager::Get()->TryInvokeTab(FTabId(TEXT("PluginsEditor")));
		if (const TSharedPtr<SWindow> Window = TabWindow(TEXT("PluginsEditor")))
		{
			Window->Resize(FVector2D(1500.0, 900.0));
		}
		Next();
		return false;

	case 1:
		if (Elapsed < 3.0)
		{
			return false;
		}
		if (const TSharedPtr<SWindow> Window = TabWindow(TEXT("PluginsEditor")))
		{
			const TSharedPtr<SWidget> Search = FeelManualCapture::FindWidget(Window.ToSharedRef(), TEXT("SSearchBox"));
			if (Search.IsValid())
			{
				StaticCastSharedPtr<SSearchBox>(Search)->SetText(FText::FromString(TEXT("FeelKit")));
			}
			else
			{
				Test->AddWarning(TEXT("MANUALSHOT Guide_Plugins: no search box"));
			}
		}
		Next();
		return false;

	case 2:
		if (Elapsed < 3.0)
		{
			return false;
		}
		if (!SaveWindow(TabWindow(TEXT("PluginsEditor")), TEXT("Guide_Plugins"), Density, Test))
		{
			return false;
		}
		CloseTab(TEXT("PluginsEditor"));
		{
			UContentBrowserSettings* Browser = GetMutableDefault<UContentBrowserSettings>();
			Run->bPreviousPluginFolders = Browser->GetDisplayPluginFolders();
			Browser->SetDisplayPluginFolders(true);
			Browser->PostEditChange();
			const TSharedPtr<SDockTab> Tab = FGlobalTabmanager::Get()->TryInvokeTab(FTabId(TEXT("ContentBrowserTab2")));
			if (const TSharedPtr<SWindow> Window = Tab.IsValid() ? Tab->GetParentWindow() : nullptr)
			{
				Window->Resize(FVector2D(1500.0, 820.0));
			}
			IContentBrowserSingleton& ContentBrowser = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser")).Get();
			ContentBrowser.SyncBrowserToFolders(TArray<FString>{ TEXT("/FeelKit") }, false, true, TEXT("ContentBrowserTab2"));
		}
		Next();
		return false;

	case 3:
		if (Elapsed < 4.0)
		{
			return false;
		}
		if (!SaveWindow(TabWindow(TEXT("ContentBrowserTab2")), TEXT("Guide_FeelKitContent"), Density, Test))
		{
			return false;
		}
		if (!OpenContentBrowserSettings(TabWindow(TEXT("ContentBrowserTab2"))))
		{
			Test->AddWarning(TEXT("MANUALSHOT Guide_ContentBrowserSettings: no Settings button"));
		}
		Next();
		return false;

	case 4:
		if (Elapsed < 2.0)
		{
			return false;
		}
		if (!SaveWindow(TabWindow(TEXT("ContentBrowserTab2")), TEXT("Guide_ContentBrowserSettings"), Density, Test))
		{
			return false;
		}
		FSlateApplication::Get().DismissAllMenus();
		CloseTab(TEXT("ContentBrowserTab2"));
		if (!HeavyHit)
		{
			Test->AddError(TEXT("The project copy /Game/HeavyHit is missing."));
			return true;
		}
		OpenEditor(HeavyHit);
		Next();
		return false;

	case 5:
		if (Elapsed < 1.0)
		{
			return false;
		}
		CloseIntensityTab(HeavyHit);
		if (const TSharedPtr<FFeelRecipeEditorState> State = FFeelRecipeEditorState::FindOpenState(HeavyHit))
		{
			State->SetTime(0.05f);
			State->SetSelectedTrack(1);
		}
		Next();
		return false;

	case 6:
		if (Elapsed < 6.0)
		{
			return false;
		}
		if (!SaveWindow(FeelManualCapture::AssetEditorWindow(HeavyHit), TEXT("Guide_TrackSelected"), Density, Test))
		{
			return false;
		}
		if (!OpenMenuWithText(FeelManualCapture::AssetEditorWindow(HeavyHit), TEXT("Comfort:"), true))
		{
			Test->AddWarning(TEXT("MANUALSHOT Guide_ComfortMenu: no Comfort button"));
		}
		Next();
		return false;

	case 7:
		if (Elapsed < 2.0)
		{
			return false;
		}
		if (!SaveWindow(FeelManualCapture::AssetEditorWindow(HeavyHit), TEXT("Guide_ComfortMenu"), Density, Test))
		{
			return false;
		}
		FSlateApplication::Get().DismissAllMenus();
		if (const TSharedPtr<FFeelRecipeEditorState> State = FFeelRecipeEditorState::FindOpenState(HeavyHit))
		{
			State->SetSelectedTrack(INDEX_NONE);
		}
		CloseEditor(HeavyHit);
		{
			// An editable copy of a sustained library recipe, so the picture has no read-only banner.
			UFeelRecipe* Library = LoadObject<UFeelRecipe>(nullptr, TEXT("/FeelKit/Library/Power/FR_Power_ChargeUp.FR_Power_ChargeUp"));
			if (!Library)
			{
				Test->AddError(TEXT("FR_Power_ChargeUp is missing."));
				return true;
			}
			if (!Library->FullReleaseRecipe)
			{
				// The pictures show the On Full Release row; an asset imported before that setting existed shows the old layout.
				Test->AddError(TEXT("FR_Power_ChargeUp has no On Full Release recipe, so the asset is older than its JSON. Re-import Library/Power/FR_Power_ChargeUp.json with Tools/Unreal/import_recipe_json.py, then take the pictures again."));
				return true;
			}
			UPackage* Package = CreatePackage(TEXT("/Temp/FeelKitManual/ChargeUp"));
			UFeelRecipe* Copy = DuplicateObject<UFeelRecipe>(Library, Package, TEXT("ChargeUp"));
			Copy->SetFlags(RF_Public | RF_Standalone | RF_Transactional);
			Run->Sustained = Copy;
			OpenEditor(Copy);
		}
		Next();
		return false;

	case 8:
		if (Elapsed < 1.0)
		{
			return false;
		}
		CloseIntensityTab(Run->Sustained.Get());
		if (const TSharedPtr<FFeelRecipeEditorState> State = FFeelRecipeEditorState::FindOpenState(Run->Sustained.Get()))
		{
			State->SetLooping(true);
			State->TogglePlay();
		}
		Next();
		return false;

	case 9:
		// Several loops of the sustain region in, so the playhead is inside it and Release is enabled.
		if (Elapsed < 5.0)
		{
			return false;
		}
		if (!SaveWindow(FeelManualCapture::AssetEditorWindow(Run->Sustained.Get()), TEXT("Guide_Sustain"), Density, Test))
		{
			return false;
		}
		if (const TSharedPtr<FFeelRecipeEditorState> State = FFeelRecipeEditorState::FindOpenState(Run->Sustained.Get()))
		{
			State->Stop();
		}
		CloseEditor(Run->Sustained.Get());
		{
			// A real save of a recipe with a problem, so Unreal's save validation fills the Asset Check page.
			UPackage* Package = CreatePackage(TEXT("/Game/FeelKitManualTemp/HitCheck"));
			UFeelRecipe* Recipe = NewObject<UFeelRecipe>(Package, TEXT("HitCheck"), RF_Public | RF_Standalone | RF_Transactional);
			FFeelTrack Track;
			Track.Step = NewObject<UFeelStep_PlaySound>(Recipe, NAME_None, RF_Transactional);
			Track.Channel = Track.Step->GetDefaultChannel();
			Recipe->Tracks.Add(Track);
			FAssetRegistryModule::AssetCreated(Recipe);
			Package->MarkPackageDirty();
			Run->Invalid = Recipe;
			UEditorLoadingAndSavingUtils::SavePackages({ Package }, false);
		}
		Next();
		return false;

	case 10:
		if (Elapsed < 3.0)
		{
			return false;
		}
		FGlobalTabmanager::Get()->TryInvokeTab(FTabId(TEXT("MessageLog")));
		if (const TSharedPtr<SWindow> Window = TabWindow(TEXT("MessageLog")))
		{
			if (Window != FGlobalTabmanager::Get()->GetRootWindow())
			{
				Window->Resize(FVector2D(1300.0, 600.0));
			}
		}
		Next();
		return false;

	case 11:
		if (Elapsed < 2.0)
		{
			return false;
		}
		if (!SaveWindow(TabWindow(TEXT("MessageLog")), TEXT("Guide_Validation"), Density, Test))
		{
			return false;
		}
		CloseTab(TEXT("MessageLog"));
		if (UFeelRecipe* Recipe = Run->Invalid.Get())
		{
			ObjectTools::ForceDeleteObjects({ Recipe }, false);
		}
		Next();
		return false;

	default:
		if (Elapsed < 2.0)
		{
			return false;
		}
		{
			UContentBrowserSettings* Browser = GetMutableDefault<UContentBrowserSettings>();
			Browser->SetDisplayPluginFolders(Run->bPreviousPluginFolders);
			Browser->PostEditChange();
		}
		FSlateApplication::Get().SetApplicationScale(Run->PreviousScale);
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelGuideShotsDiagnostic, "DiagFeel.ManualShotsGuide", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FFeelGuideShotsDiagnostic::RunTest(const FString& Parameters)
{
	if (!FApp::CanEverRender() || !FSlateApplication::IsInitialized() || !GEditor)
	{
		AddError(TEXT("Needs a rendering session."));
		return false;
	}
	const TSharedRef<FeelGuideShots::FRun> Run = MakeShared<FeelGuideShots::FRun>();
	Run->PreviousScale = FSlateApplication::Get().GetApplicationScale();
	// The deliberate validation error of the Asset Check picture.
	AddExpectedMessage(TEXT("Play Sound has no sound"), EAutomationExpectedMessageFlags::Contains, 0);
	FSlateApplication::Get().SetApplicationScale(1.25f);
	ADD_LATENT_AUTOMATION_COMMAND(FFeelGuideShotsCommand(Run, this));
	return true;
}

#endif
