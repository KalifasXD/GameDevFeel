// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Editor.h"
#include "FeelManualCapture.h"
#include "FeelRecipe.h"
#include "FeelRecipeEditorState.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "Misc/App.h"
#include "Misc/ConfigCacheIni.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "Sound/SoundBase.h"
#include "Steps/FeelStep_Meta.h"
#include "Steps/FeelStep_PlaySound.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "UObject/Package.h"
#include "Widgets/Docking/SDockTab.h"

/**
 * Diagnostic (filter DiagFeel): the pictures of the manual's chapter on parameters and variation, saved to
 * Saved/FeelKit/Manual/Var_*.png at twice the screen's pixels and 1.25x application scale: a transient copy of
 * FR_Impact_ScalableHit with a track selected and its Parameters and Parameter Mappings open, and a transient recipe
 * with a Random Choice track of three Play Sound options. The details views' remembered expansion is changed for the
 * pictures and put back. Needs a rendering session.
 */
namespace FeelVariationShots
{
	constexpr float Density = 2.0f;

	const TCHAR* Categories[] = { TEXT("FeelRecipe.Recipe"), TEXT("FeelRecipe.Parameters"), TEXT("FeelRecipe.Sustain"), TEXT("FeelRecipe.Library"),
		TEXT("FeelRecipe.Preview"), TEXT("FeelRecipe.Parameter Mappings"), TEXT("FeelRecipe.Randomness"), TEXT("FeelRecipe.Conditions"),
		TEXT("FeelRecipe.Comfort"), TEXT("FeelRecipe.Advanced") };

	struct FRun
	{
		int32 Stage = 0;
		double StageStart = 0.0;
		float PreviousScale = 1.0f;
		TMap<FString, FString> SavedCategories;
		TArray<FString> SavedProperties;
		TWeakObjectPtr<UFeelRecipe> Scalable;
		TWeakObjectPtr<UFeelRecipe> Choice;
		TSharedPtr<SWindow> Floating;
		TArray<FString> SavedChoiceProperties;
	};

	void Save(FRun& Run)
	{
		for (const TCHAR* Key : Categories)
		{
			FString Value;
			GConfig->GetString(TEXT("DetailCategories"), Key, Value, GEditorPerProjectIni);
			Run.SavedCategories.Add(Key, Value);
		}
		GConfig->GetSingleLineArray(TEXT("DetailPropertyExpansion"), TEXT("FeelRecipe"), Run.SavedProperties, GEditorPerProjectIni);
	}

	void Restore(const FRun& Run)
	{
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
		TArray<FString> Quoted;
		for (const FString& Item : Run.SavedProperties)
		{
			Quoted.Add(FString::Printf(TEXT("\"%s\""), *Item));
		}
		GConfig->SetSingleLineArray(TEXT("DetailPropertyExpansion"), TEXT("FeelRecipe"), Quoted, GEditorPerProjectIni);
	}

	/** Opens only the named categories and properties the next time a recipe's details are built. */
	void Expand(const TArray<FString>& OpenCategories, const TArray<FString>& OpenProperties)
	{
		for (const TCHAR* Key : Categories)
		{
			GConfig->SetBool(TEXT("DetailCategories"), Key, OpenCategories.Contains(Key), GEditorPerProjectIni);
		}
		TArray<FString> Quoted;
		for (const FString& Item : OpenProperties)
		{
			Quoted.Add(FString::Printf(TEXT("\"%s\""), *Item));
		}
		GConfig->SetSingleLineArray(TEXT("DetailPropertyExpansion"), TEXT("FeelRecipe"), Quoted, GEditorPerProjectIni);
	}

	void Open(UFeelRecipe* Recipe, int32 SelectedTrack)
	{
		UAssetEditorSubsystem* Editors = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
		Editors->OpenEditorForAsset(Recipe);
		if (const TSharedPtr<SWindow> Window = FeelManualCapture::AssetEditorWindow(Recipe))
		{
			Window->Resize(FVector2D(1900.0, 1060.0));
		}
		IAssetEditorInstance* Editor = Editors->FindEditorForAsset(Recipe, false);
		const TSharedPtr<FTabManager> Tabs = Editor ? Editor->GetAssociatedTabManager() : nullptr;
		if (const TSharedPtr<SDockTab> Intensity = Tabs.IsValid() ? Tabs->FindExistingLiveTab(FTabId(TEXT("FeelRecipeEditor_Intensity"))) : nullptr)
		{
			Intensity->RequestCloseTab();
		}
		if (const TSharedPtr<FFeelRecipeEditorState> State = FFeelRecipeEditorState::FindOpenState(Recipe))
		{
			State->SetSelectedTrack(SelectedTrack);
		}
	}

	UFeelRecipe* BuildChoiceRecipe()
	{
		UPackage* Package = CreatePackage(TEXT("/Temp/FeelKitManualVariation/ImpactSounds"));
		UFeelRecipe* Recipe = NewObject<UFeelRecipe>(Package, TEXT("ImpactSounds"), RF_Public | RF_Standalone | RF_Transactional);
		UFeelStep_RandomChoice* Choice = NewObject<UFeelStep_RandomChoice>(Recipe, NAME_None, RF_Transactional);
		const TCHAR* Sounds[] = {
			TEXT("/FeelKit/Samples/Sounds/S_FK_Impact_Thud.S_FK_Impact_Thud"),
			TEXT("/FeelKit/Samples/Sounds/S_FK_Impact_Bullet.S_FK_Impact_Bullet"),
			TEXT("/FeelKit/Samples/Sounds/S_FK_Body_Hit.S_FK_Body_Hit") };
		const float Weights[] = { 2.0f, 1.0f, 1.0f };
		for (int32 Index = 0; Index < 3; ++Index)
		{
			UFeelStep_PlaySound* Sound = NewObject<UFeelStep_PlaySound>(Choice, NAME_None, RF_Transactional);
			Sound->Sound = LoadObject<USoundBase>(nullptr, Sounds[Index]);
			FFeelRandomChoiceOption Option;
			Option.Step = Sound;
			Option.Weight = Weights[Index];
			Choice->Options.Add(Option);
		}
		FFeelTrack Track;
		Track.Step = Choice;
		Track.Channel = Choice->GetDefaultChannel();
		Track.Duration = 0.5f;
		Recipe->Tracks.Add(Track);
		return Recipe;
	}
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FFeelVariationShotsCommand, TSharedRef<FeelVariationShots::FRun>, Run, FAutomationTestBase*, Test);

bool FFeelVariationShotsCommand::Update()
{
	using namespace FeelVariationShots;
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
	UAssetEditorSubsystem* Editors = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();

	switch (Run->Stage)
	{
	case 0:
	{
		UFeelRecipe* Library = LoadObject<UFeelRecipe>(nullptr, TEXT("/FeelKit/Library/Impact/FR_Impact_ScalableHit.FR_Impact_ScalableHit"));
		if (!Library)
		{
			Test->AddError(TEXT("FR_Impact_ScalableHit is missing."));
			return true;
		}
		Save(*Run);
		Expand({ TEXT("FeelRecipe.Parameters"), TEXT("FeelRecipe.Parameter Mappings") },
			{ TEXT("Object.Parameters.Parameters"), TEXT("Object.Parameters.Parameters.Parameters[0]"),
			  TEXT("Object.Parameter Mappings.ParameterMappings"), TEXT("Object.Parameter Mappings.ParameterMappings.ParameterMappings[0]") });
		UPackage* Package = CreatePackage(TEXT("/Temp/FeelKitManualVariation/ScalableHit"));
		UFeelRecipe* Copy = DuplicateObject<UFeelRecipe>(Library, Package, TEXT("ScalableHit"));
		Copy->SetFlags(RF_Public | RF_Standalone | RF_Transactional);
		Run->Scalable = Copy;
		// The shake track: the second track of the recipe.
		Open(Copy, FMath::Min(1, Copy->Tracks.Num() - 1));
		Next();
		return false;
	}

	case 1:
		if (Elapsed < 5.0)
		{
			return false;
		}
		if (!FeelManualCapture::SaveWindow(FeelManualCapture::AssetEditorWindow(Run->Scalable.Get()), TEXT("Var_Parameters"), Density, Test))
		{
			return false;
		}
		Editors->CloseAllEditorsForAsset(Run->Scalable.Get());
		Expand({ TEXT("FeelRecipe.Recipe") }, {});
		{
			UFeelRecipe* Choice = BuildChoiceRecipe();
			Run->Choice = Choice;
			Open(Choice, 0);
		}
		Next();
		return false;

	case 2:
		if (Elapsed < 5.0)
		{
			return false;
		}
		if (!FeelManualCapture::SaveWindow(FeelManualCapture::AssetEditorWindow(Run->Choice.Get()), TEXT("Var_RandomChoice"), Density, Test))
		{
			return false;
		}
		Editors->CloseAllEditorsForAsset(Run->Choice.Get());
		{
			// The step's own settings, with its options open.
			GConfig->GetSingleLineArray(TEXT("DetailPropertyExpansion"), TEXT("FeelStep_RandomChoice"), Run->SavedChoiceProperties, GEditorPerProjectIni);
			GConfig->SetSingleLineArray(TEXT("DetailPropertyExpansion"), TEXT("FeelStep_RandomChoice"), {
				TEXT("\"Object.Choice.Options\""), TEXT("\"Object.Choice.Options.Options[0]\""), TEXT("\"Object.Choice.Options.Options[1]\""),
				TEXT("\"Object.Choice.Options.Options[2]\"") }, GEditorPerProjectIni);
			UFeelRecipe* Recipe = Run->Choice.Get();
			UObject* Step = Recipe && Recipe->Tracks.Num() > 0 ? Recipe->Tracks[0].Step.Get() : nullptr;
			if (Step)
			{
				FPropertyEditorModule& PropertyEditor = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
				const TSharedRef<SWindow> Window = PropertyEditor.CreateFloatingDetailsView({ Step }, false);
				Window->Resize(FVector2D(1000.0, 520.0));
				Run->Floating = Window;
			}
		}
		Next();
		return false;

	case 3:
		if (Elapsed < 3.0)
		{
			return false;
		}
		if (!FeelManualCapture::SaveWindow(Run->Floating, TEXT("Var_ChoiceOptions"), Density, Test))
		{
			return false;
		}
		if (Run->Floating.IsValid())
		{
			Run->Floating->RequestDestroyWindow();
			Run->Floating.Reset();
		}
		{
			TArray<FString> Quoted;
			for (const FString& Item : Run->SavedChoiceProperties)
			{
				Quoted.Add(FString::Printf(TEXT("\"%s\""), *Item));
			}
			GConfig->SetSingleLineArray(TEXT("DetailPropertyExpansion"), TEXT("FeelStep_RandomChoice"), Quoted, GEditorPerProjectIni);
		}
		Next();
		return false;

	default:
		if (Elapsed < 1.0)
		{
			return false;
		}
		Restore(*Run);
		FSlateApplication::Get().SetApplicationScale(Run->PreviousScale);
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelVariationShotsDiagnostic, "DiagFeel.ManualShotsVariation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FFeelVariationShotsDiagnostic::RunTest(const FString& Parameters)
{
	if (!FApp::CanEverRender() || !FSlateApplication::IsInitialized() || !GEditor)
	{
		AddError(TEXT("Needs a rendering session."));
		return false;
	}
	const TSharedRef<FeelVariationShots::FRun> Run = MakeShared<FeelVariationShots::FRun>();
	Run->PreviousScale = FSlateApplication::Get().GetApplicationScale();
	FSlateApplication::Get().SetApplicationScale(1.25f);
	ADD_LATENT_AUTOMATION_COMMAND(FFeelVariationShotsCommand(Run, this));
	return true;
}

#endif
