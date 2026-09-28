// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Editor.h"
#include "FeelManualCapture.h"
#include "FeelMap.h"
#include "FeelRecipe.h"
#include "Framework/Application/SlateApplication.h"
#include "GameplayTagContainer.h"
#include "Misc/App.h"
#include "Misc/ConfigCacheIni.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "UObject/Package.h"

/**
 * Diagnostic (filter DiagFeel): the Feel Map picture of the manual's chapter 2, saved to
 * Saved/FeelKit/Manual/Concepts_FeelMap.png at twice the screen's pixels: a Feel Map in a transient package with four rows,
 * two of them made more specific by the project tags Hit.Heavy and Hit.Critical, in a floating Details window at 1.0x
 * application scale so all four rows fit. The two tags are not FeelKit's: the runner adds them to the project's gameplay
 * tag list for the run and puts the list back afterwards (Tools/Run/manual_concepts_shot.ps1). The details view's
 * remembered expansion is put back as well. Needs a rendering session.
 */
namespace FeelConceptsShots
{
	constexpr float Density = 2.0f;

	struct FRun
	{
		int32 Stage = 0;
		double StageStart = 0.0;
		float PreviousScale = 1.0f;
		TArray<FString> SavedExpansion;
		TSharedPtr<SWindow> Floating;
	};

	FFeelMapEntry Row(const TCHAR* Event, const TCHAR* Required, const TCHAR* RecipePath)
	{
		FFeelMapEntry Entry;
		Entry.Event = FGameplayTag::RequestGameplayTag(Event);
		if (Required)
		{
			Entry.RequiredTags.AddTag(FGameplayTag::RequestGameplayTag(Required));
		}
		Entry.Recipe = LoadObject<UFeelRecipe>(nullptr, RecipePath);
		return Entry;
	}
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FFeelConceptsShotsCommand, TSharedRef<FeelConceptsShots::FRun>, Run, FAutomationTestBase*, Test);

bool FFeelConceptsShotsCommand::Update()
{
	using namespace FeelConceptsShots;
	const double Now = FPlatformTime::Seconds();
	if (Run->StageStart == 0.0)
	{
		Run->StageStart = Now;
	}
	const double Elapsed = Now - Run->StageStart;

	switch (Run->Stage)
	{
	case 0:
	{
		if (!FGameplayTag::RequestGameplayTag(TEXT("Hit.Heavy"), false).IsValid() || !FGameplayTag::RequestGameplayTag(TEXT("Hit.Critical"), false).IsValid())
		{
			Test->AddError(TEXT("The tags Hit.Heavy and Hit.Critical are missing; run the picture through manual_concepts_shot.ps1."));
			return true;
		}
		UPackage* Package = CreatePackage(TEXT("/Temp/FeelKitManualConcepts/FM_Combat"));
		UFeelMap* Map = NewObject<UFeelMap>(Package, TEXT("FM_Combat"), RF_Public | RF_Standalone | RF_Transactional);
		Map->Entries.Add(Row(TEXT("Feel.Event.Hit.Landed"), nullptr, TEXT("/FeelKit/Library/Impact/FR_Impact_LightHit.FR_Impact_LightHit")));
		Map->Entries.Add(Row(TEXT("Feel.Event.Hit.Landed"), TEXT("Hit.Heavy"), TEXT("/FeelKit/Library/Impact/FR_Impact_HeavyHit.FR_Impact_HeavyHit")));
		Map->Entries.Add(Row(TEXT("Feel.Event.Hit.Landed"), TEXT("Hit.Critical"), TEXT("/FeelKit/Library/Impact/FR_Impact_CriticalHit.FR_Impact_CriticalHit")));
		Map->Entries.Add(Row(TEXT("Feel.Event.Hit.Received"), nullptr, TEXT("/FeelKit/Library/Danger/FR_Danger_DirectionalDamage.FR_Danger_DirectionalDamage")));

		GConfig->GetSingleLineArray(TEXT("DetailPropertyExpansion"), TEXT("FeelMap"), Run->SavedExpansion, GEditorPerProjectIni);
		GConfig->SetSingleLineArray(TEXT("DetailPropertyExpansion"), TEXT("FeelMap"), { TEXT("\"Object.Feel Map.Entries\""),
			TEXT("\"Object.Feel Map.Entries.Entries[0]\""), TEXT("\"Object.Feel Map.Entries.Entries[1]\""),
			TEXT("\"Object.Feel Map.Entries.Entries[2]\""), TEXT("\"Object.Feel Map.Entries.Entries[3]\"") }, GEditorPerProjectIni);

		FPropertyEditorModule& Editor = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
		Run->Floating = Editor.CreateFloatingDetailsView({ Map }, false);
		Run->Floating->Resize(FVector2D(1000.0, 1050.0));
		++Run->Stage;
		Run->StageStart = Now;
		return false;
	}

	case 1:
		if (Elapsed < 3.0 || !FeelManualCapture::SaveWindow(Run->Floating, TEXT("Concepts_FeelMap"), Density, Test))
		{
			return false;
		}
		Run->Floating->RequestDestroyWindow();
		Run->Floating.Reset();
		GConfig->SetSingleLineArray(TEXT("DetailPropertyExpansion"), TEXT("FeelMap"), Run->SavedExpansion, GEditorPerProjectIni);
		FSlateApplication::Get().SetApplicationScale(Run->PreviousScale);
		return true;

	default:
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelConceptsShotsDiagnostic, "DiagFeel.ManualShotsConcepts", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FFeelConceptsShotsDiagnostic::RunTest(const FString& Parameters)
{
	if (!FApp::CanEverRender() || !FSlateApplication::IsInitialized() || !GEditor)
	{
		AddError(TEXT("Needs a rendering session."));
		return false;
	}
	const TSharedRef<FeelConceptsShots::FRun> Run = MakeShared<FeelConceptsShots::FRun>();
	Run->PreviousScale = FSlateApplication::Get().GetApplicationScale();
	FSlateApplication::Get().SetApplicationScale(1.0f);
	ADD_LATENT_AUTOMATION_COMMAND(FFeelConceptsShotsCommand(Run, this));
	return true;
}

#endif
