// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "EdGraph/EdGraph.h"
#include "Editor.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/GameViewportClient.h"
#include "FeelBlueprintLibrary.h"
#include "FeelComfortSubsystem.h"
#include "FeelManualCapture.h"
#include "FeelRecipe.h"
#include "FeelRecipeEditorState.h"
#include "FeelSettings.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "ISettingsModule.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/App.h"
#include "Misc/ConfigCacheIni.h"
#include "Modules/ModuleManager.h"
#include "SGraphPanel.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Steps/FeelStep_CameraPunch.h"
#include "Steps/FeelStep_Hitstop.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "UObject/Package.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Views/STableViewBase.h"

/**
 * Diagnostic (filter DiagFeel): the pictures of the manual's reference chapters, saved to Saved/FeelKit/Manual/Ref_*.png
 * at twice the screen's pixels and 1.25x application scale: the + Track menu, a recipe with a track that has no preview,
 * the Blueprint action menu filtered by "Feel", the FeelKit pages of Project Settings and Editor Preferences with chosen
 * categories open, and showdebug feel while two recipes play. The categories are opened through the details views'
 * remembered expansion (EditorPerProjectUserSettings), which is put back afterwards. Needs a rendering session and the
 * /Game/HeavyHit copy.
 */
namespace FeelReferenceShots
{
	constexpr float Density = 2.0f;

	struct FRun
	{
		int32 Stage = 0;
		double StageStart = 0.0;
		float PreviousScale = 1.0f;
		TWeakObjectPtr<UFeelRecipe> Recipe;
		TWeakObjectPtr<UBlueprint> Blueprint;
		TMap<FString, FString> SavedCategories;
		TArray<FString> SavedProperties;
		TArray<FFeelAccumulatorDefinition> SavedAccumulators;
		bool bSavedState = false;
	};

	const TCHAR* CategoryKeys[] = {
		TEXT("FeelSettings.Camera"), TEXT("FeelSettings.Playback"), TEXT("FeelSettings.Accumulator"), TEXT("FeelSettings.Events"),
		TEXT("FeelSettings.Comfort"), TEXT("FeelSettings.Comfort.Presets"), TEXT("FeelSettings.Comfort.Storage"),
		TEXT("FeelEditorSettings.Timeline"), TEXT("FeelEditorSettings.Library"),
	};

	void SaveState(FRun& Run)
	{
		for (const TCHAR* Key : CategoryKeys)
		{
			FString Value;
			GConfig->GetString(TEXT("DetailCategories"), Key, Value, GEditorPerProjectIni);
			Run.SavedCategories.Add(Key, Value);
		}
		GConfig->GetSingleLineArray(TEXT("DetailPropertyExpansion"), TEXT("FeelSettings"), Run.SavedProperties, GEditorPerProjectIni);
		// This project defines four accumulators for its demos; a new project has none. The picture shows one example.
		UFeelSettings* Settings = GetMutableDefault<UFeelSettings>();
		Run.SavedAccumulators = Settings->Accumulators;
		Settings->Accumulators.SetNum(FMath::Min(1, Settings->Accumulators.Num()));
		Run.bSavedState = true;
	}

	void RestoreState(const FRun& Run)
	{
		if (!Run.bSavedState)
		{
			return;
		}
		for (const TPair<FString, FString>& Pair : Run.SavedCategories)
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
		GConfig->SetSingleLineArray(TEXT("DetailPropertyExpansion"), TEXT("FeelSettings"), Run.SavedProperties, GEditorPerProjectIni);
		GetMutableDefault<UFeelSettings>()->Accumulators = Run.SavedAccumulators;
	}

	/** Opens exactly the named categories and properties the next time a FeelKit settings page is built. */
	void Expand(const TArray<FString>& OpenCategories, const TArray<FString>& OpenProperties)
	{
		for (const TCHAR* Key : CategoryKeys)
		{
			GConfig->SetBool(TEXT("DetailCategories"), Key, OpenCategories.Contains(Key), GEditorPerProjectIni);
		}
		GConfig->SetSingleLineArray(TEXT("DetailPropertyExpansion"), TEXT("FeelSettings"), OpenProperties, GEditorPerProjectIni);
	}

	FName SettingsTab(bool bProject)
	{
		return bProject ? FName(TEXT("ProjectSettings")) : FName(TEXT("EditorSettings"));
	}

	void ShowSettings(bool bProject)
	{
		ISettingsModule& Settings = FModuleManager::LoadModuleChecked<ISettingsModule>(TEXT("Settings"));
		Settings.ShowViewer(bProject ? TEXT("Project") : TEXT("Editor"), TEXT("Plugins"), bProject ? TEXT("FeelSettings") : TEXT("FeelEditorSettings"));
		const TSharedPtr<SDockTab> Tab = FGlobalTabmanager::Get()->FindExistingLiveTab(FTabId(SettingsTab(bProject)));
		if (const TSharedPtr<SWindow> Window = Tab.IsValid() ? Tab->GetParentWindow() : nullptr)
		{
			Window->Resize(FVector2D(1500.0, 1060.0));
		}
	}

	TSharedPtr<SWindow> SettingsWindow(bool bProject)
	{
		const TSharedPtr<SDockTab> Tab = FGlobalTabmanager::Get()->FindExistingLiveTab(FTabId(SettingsTab(bProject)));
		return Tab.IsValid() ? Tab->GetParentWindow() : nullptr;
	}

	void CloseSettings(bool bProject)
	{
		if (const TSharedPtr<SDockTab> Tab = FGlobalTabmanager::Get()->FindExistingLiveTab(FTabId(SettingsTab(bProject))))
		{
			Tab->RequestCloseTab();
		}
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

	void TypeText(const FString& Text)
	{
		for (const TCHAR Character : Text)
		{
			FSlateApplication::Get().ProcessKeyCharEvent(FCharacterEvent(Character, FModifierKeysState(), 0, false));
		}
	}
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FFeelReferenceShotsCommand, TSharedRef<FeelReferenceShots::FRun>, Run, FAutomationTestBase*, Test);

bool FFeelReferenceShotsCommand::Update()
{
	using namespace FeelReferenceShots;
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
		if (!HeavyHit)
		{
			Test->AddError(TEXT("The project copy /Game/HeavyHit is missing."));
			return true;
		}
		OpenEditor(HeavyHit);
		Next();
		return false;

	case 1:
		if (Elapsed < 6.0)
		{
			return false;
		}
		if (const TSharedPtr<SWindow> Window = FeelManualCapture::AssetEditorWindow(HeavyHit))
		{
			// All tracks in view: the Intensity tab would take the lower half of the timeline.
			IAssetEditorInstance* Editor = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(HeavyHit, false);
			const TSharedPtr<FTabManager> Tabs = Editor ? Editor->GetAssociatedTabManager() : nullptr;
			if (const TSharedPtr<SDockTab> Intensity = Tabs.IsValid() ? Tabs->FindExistingLiveTab(FTabId(TEXT("FeelRecipeEditor_Intensity"))) : nullptr)
			{
				Intensity->RequestCloseTab();
			}
			// The + Track button is a combo button; opening it shows the step menu as a child window.
			const TSharedPtr<SWidget> Button = FeelManualCapture::FindWidget(Window.ToSharedRef(), TEXT("SPositiveActionButton"));
			const TSharedPtr<SWidget> Combo = Button.IsValid() ? FeelManualCapture::FindWidget(Button.ToSharedRef(), TEXT("SComboButton")) : nullptr;
			if (Combo.IsValid())
			{
				StaticCastSharedPtr<SComboButton>(Combo)->SetIsOpen(true);
			}
			else
			{
				Test->AddWarning(TEXT("MANUALSHOT Ref_TrackMenu: the + Track button was not found"));
			}
		}
		Next();
		return false;

	case 2:
		if (Elapsed < 2.0)
		{
			return false;
		}
		if (!SaveWindow(FeelManualCapture::AssetEditorWindow(HeavyHit), TEXT("Ref_TrackMenu"), Density, Test))
		{
			return false;
		}
		FSlateApplication::Get().DismissAllMenus();
		CloseEditor(HeavyHit);
		{
			// A recipe with one track that previews and one that cannot.
			UPackage* Package = CreatePackage(TEXT("/Temp/FeelKitManual/HitstopAndPunch"));
			UFeelRecipe* Recipe = NewObject<UFeelRecipe>(Package, TEXT("HitstopAndPunch"), RF_Public | RF_Standalone | RF_Transactional);
			Run->Recipe = Recipe;
			OpenEditor(Recipe);
			if (const TSharedPtr<FFeelRecipeEditorState> State = FFeelRecipeEditorState::FindOpenState(Recipe))
			{
				State->AddTrack(UFeelStep_GlobalHitstop::StaticClass());
				State->AddTrack(UFeelStep_CameraPunch::StaticClass());
				State->SetSelectedTrack(INDEX_NONE);
			}
		}
		Next();
		return false;

	case 3:
		if (Elapsed < 6.0)
		{
			return false;
		}
		if (!SaveWindow(FeelManualCapture::AssetEditorWindow(Run->Recipe.Get()), TEXT("Ref_NoPreview"), Density, Test))
		{
			return false;
		}
		CloseEditor(Run->Recipe.Get());
		{
			UPackage* Package = CreatePackage(TEXT("/Temp/FeelKitManualReference/BP_ThirdPersonCharacter"));
			UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(ACharacter::StaticClass(), Package, TEXT("BP_ThirdPersonCharacter"),
				BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
			if (UEdGraph* Graph = FBlueprintEditorUtils::FindEventGraph(Blueprint))
			{
				TArray<UEdGraphNode*> Existing = Graph->Nodes;
				for (UEdGraphNode* Node : Existing)
				{
					Graph->RemoveNode(Node);
				}
			}
			FKismetEditorUtilities::CompileBlueprint(Blueprint);
			Run->Blueprint = Blueprint;
			OpenEditor(Blueprint);
			if (UEdGraph* Graph = FBlueprintEditorUtils::FindEventGraph(Blueprint))
			{
				FKismetEditorUtilities::BringKismetToFocusAttentionOnObject(Graph);
			}
		}
		Next();
		return false;

	case 4:
		if (Elapsed < 4.0)
		{
			return false;
		}
		if (const TSharedPtr<SWindow> Window = FeelManualCapture::AssetEditorWindow(Run->Blueprint.Get()))
		{
			const TSharedPtr<SGraphPanel> Panel = StaticCastSharedPtr<SGraphPanel>(FeelManualCapture::FindWidget(Window.ToSharedRef(), TEXT("SGraphPanel")));
			if (Panel.IsValid())
			{
				const FGeometry& Geometry = Panel->GetTickSpaceGeometry();
				const FVector2D Where = FVector2D(Geometry.LocalToAbsolute(Geometry.GetLocalSize() * FVector2f(0.42f, 0.18f)));
				const TSharedPtr<SWidget> Search = Panel->SummonContextMenu(Where, FVector2D::ZeroVector, nullptr, nullptr, TArray<UEdGraphPin*>());
				if (Search.IsValid())
				{
					FSlateApplication::Get().SetKeyboardFocus(Search, EFocusCause::SetDirectly);
					TypeText(TEXT("Feel"));
				}
			}
			else
			{
				Test->AddWarning(TEXT("MANUALSHOT Ref_BlueprintMenu: no graph panel"));
			}
		}
		Next();
		return false;

	case 5:
		if (Elapsed < 2.5)
		{
			return false;
		}
		if (const TSharedPtr<SWindow> Window = FeelManualCapture::AssetEditorWindow(Run->Blueprint.Get()))
		{
			// Typing selects the last match and scrolls to it; the picture shows the list from the top.
			for (const TSharedRef<SWindow>& Menu : Window->GetChildWindows())
			{
				if (const TSharedPtr<SWidget> List = FeelManualCapture::FindWidgetStartingWith(Menu, TEXT("STreeView")))
				{
					StaticCastSharedPtr<STableViewBase>(List)->ScrollToTop();
				}
			}
		}
		if (!SaveWindow(FeelManualCapture::AssetEditorWindow(Run->Blueprint.Get()), TEXT("Ref_BlueprintMenu"), Density, Test))
		{
			return false;
		}
		FSlateApplication::Get().DismissAllMenus();
		CloseEditor(Run->Blueprint.Get());
		if (FPlatformMisc::GetEnvironmentVariable(TEXT("FEELKIT_SHOTS_STOP")) == TEXT("menus"))
		{
			FSlateApplication::Get().SetApplicationScale(Run->PreviousScale);
			return true;
		}
		SaveState(*Run);
		Expand({ TEXT("FeelSettings.Camera"), TEXT("FeelSettings.Playback"), TEXT("FeelSettings.Accumulator") },
			{ TEXT("Object.Playback.Accumulators"), TEXT("Object.Playback.Accumulators.Accumulators[0]") });
		ShowSettings(true);
		Next();
		return false;

	case 6:
		if (Elapsed < 3.0)
		{
			return false;
		}
		if (!SaveWindow(SettingsWindow(true), TEXT("Ref_ProjectSettings1"), Density, Test))
		{
			return false;
		}
		CloseSettings(true);
		Expand({ TEXT("FeelSettings.Comfort") }, { TEXT("Object.Comfort.DefaultComfortScales") });
		Next();
		return false;

	case 7:
		// The tab closes on the next frame; open it again once it is gone so the new expansion is read.
		if (Elapsed < 0.5)
		{
			return false;
		}
		ShowSettings(true);
		Next();
		return false;

	case 8:
		if (Elapsed < 3.0)
		{
			return false;
		}
		if (!SaveWindow(SettingsWindow(true), TEXT("Ref_ProjectSettings2"), Density, Test))
		{
			return false;
		}
		CloseSettings(true);
		Expand({ TEXT("FeelSettings.Comfort"), TEXT("FeelSettings.Comfort.Presets"), TEXT("FeelSettings.Comfort.Storage") },
			{ TEXT("Object.Comfort.Comfort|Presets"), TEXT("Object.Comfort.Comfort|Presets.ReducedMotionPreset"), TEXT("Object.Comfort.Comfort|Storage") });
		Next();
		return false;

	case 9:
		if (Elapsed < 0.5)
		{
			return false;
		}
		ShowSettings(true);
		Next();
		return false;

	case 10:
		if (Elapsed < 3.0)
		{
			return false;
		}
		if (!SaveWindow(SettingsWindow(true), TEXT("Ref_ProjectSettings3"), Density, Test))
		{
			return false;
		}
		CloseSettings(true);
		Expand({ TEXT("FeelEditorSettings.Timeline"), TEXT("FeelEditorSettings.Library") }, {});
		ShowSettings(false);
		Next();
		return false;

	case 11:
		if (Elapsed < 3.0)
		{
			return false;
		}
		if (!SaveWindow(SettingsWindow(false), TEXT("Ref_EditorPreferences"), Density, Test))
		{
			return false;
		}
		CloseSettings(false);
		RestoreState(*Run);
		{
			ULevelEditorPlaySettings* PlaySettings = NewObject<ULevelEditorPlaySettings>(GetTransientPackage());
			PlaySettings->SetPlayNumberOfClients(1);
			PlaySettings->SetPlayNetMode(EPlayNetMode::PIE_Standalone);
			PlaySettings->LastExecutedPlayModeType = EPlayModeType::PlayMode_InEditorFloating;
			FRequestPlaySessionParams Params;
			Params.EditorPlaySettings = PlaySettings;
			GEditor->RequestPlaySession(Params);
		}
		Next();
		return false;

	case 12:
	{
		UWorld* World = FeelManualCapture::PIEWorld();
		APlayerController* Controller = World ? World->GetFirstPlayerController() : nullptr;
		if (!Controller || !Controller->GetPawn() || Elapsed < 3.0)
		{
			return Elapsed > 90.0;
		}
		Controller->ConsoleCommand(TEXT("DisableAllScreenMessages"));
		Controller->ConsoleCommand(TEXT("showdebug feel"));
		if (UGameViewportClient* Client = World->GetGameViewport())
		{
			// The debug text is drawn at the window's DPI scale and no engine screenshot contains it; at a DPI scale of 2
			// the letters have twice the pixels when the view is read from the desktop.
			const TSharedPtr<SWindow> Window = Client->GetWindow();
			if (Window.IsValid() && Window->GetNativeWindow().IsValid())
			{
				Window->GetNativeWindow()->SetDPIScaleFactor(Density);
				Client->RequestUpdateDPIScale();
			}
		}
		if (UFeelComfortSubsystem* Comfort = ULocalPlayer::GetSubsystem<UFeelComfortSubsystem>(Controller->GetLocalPlayer()))
		{
			// The comfort line shows the project defaults, not what an earlier play session saved for this player.
			Comfort->SetComfortScalesWithoutSaving(GetDefault<UFeelSettings>()->DefaultComfortScales);
		}
		const FFeelTarget Target = UFeelBlueprintLibrary::MakeFeelTargetFromActor(Controller->GetPawn());
		UFeelBlueprintLibrary::PlayFeel(World, LoadObject<UFeelRecipe>(nullptr, TEXT("/FeelKit/Library/Dread/FR_Dread_Heartbeat.FR_Dread_Heartbeat")), Target, 0.6f);
		UFeelBlueprintLibrary::PlayFeel(World, LoadObject<UFeelRecipe>(nullptr, TEXT("/FeelKit/Library/Danger/FR_Danger_LowHealth.FR_Danger_LowHealth")), Target, 0.4f);
		Next();
		return false;
	}

	case 13:
	{
		if (Elapsed < 1.5)
		{
			return false;
		}
		UWorld* World = FeelManualCapture::PIEWorld();
		FeelManualCapture::SaveGameViewFromDesktop(World ? World->GetGameViewport() : nullptr, TEXT("Ref_ShowDebug"), Test);
		Next();
		return false;
	}

	case 14:
		if (Elapsed < 2.0)
		{
			return false;
		}
		GEditor->RequestEndPlayMap();
		Next();
		return false;

	default:
		if (Elapsed < 2.0)
		{
			return false;
		}
		RestoreState(*Run);
		FSlateApplication::Get().SetApplicationScale(Run->PreviousScale);
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelReferenceShotsDiagnostic, "DiagFeel.ManualShotsReference", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FFeelReferenceShotsDiagnostic::RunTest(const FString& Parameters)
{
	if (!FApp::CanEverRender() || !FSlateApplication::IsInitialized() || !GEditor)
	{
		AddError(TEXT("Needs a rendering session."));
		return false;
	}
	const TSharedRef<FeelReferenceShots::FRun> Run = MakeShared<FeelReferenceShots::FRun>();
	Run->PreviousScale = FSlateApplication::Get().GetApplicationScale();
	FSlateApplication::Get().SetApplicationScale(1.25f);
	ADD_LATENT_AUTOMATION_COMMAND(FFeelReferenceShotsCommand(Run, this));
	return true;
}

#endif
