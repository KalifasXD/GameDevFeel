// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Curves/CurveFloat.h"
#include "FeelRecipe.h"
#include "FeelRecipeEditorState.h"
#include "FeelSaveValidationLog.h"
#include "Logging/MessageLog.h"
#include "MessageLogModule.h"
#include "Modules/ModuleManager.h"
#include "FeelSettings.h"
#include "FeelTags.h"
#include "FeelTrackClipboard.h"
#include "Steps/FeelStep_ProceduralShake.h"
#include "Steps/FeelStep_ScreenFlash.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"

#define FEEL_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelTrackClipboardTest, "FeelKit.Editor.TrackClipboard", FEEL_TEST_FLAGS)
bool FFeelTrackClipboardTest::RunTest(const FString& Parameters)
{
	TStrongObjectPtr<UFeelRecipe> Source(NewObject<UFeelRecipe>(GetTransientPackage()));
	TStrongObjectPtr<UFeelRecipe> Target(NewObject<UFeelRecipe>(GetTransientPackage()));

	UFeelStep_ScreenFlash* Flash = NewObject<UFeelStep_ScreenFlash>(Source.Get());
	Flash->MaxOpacity = 0.7f;
	{
		FFeelTrack& Track = Source->Tracks.AddDefaulted_GetRef();
		Track.Step = Flash;
		Track.StartTime = 0.2f;
		Track.Duration = 0.3f;
	}
	{
		FFeelTrack& Track = Source->Tracks.AddDefaulted_GetRef();
		Track.Step = NewObject<UFeelStep_ProceduralShake>(Source.Get());
		Track.StartTime = 0.5f;
		Track.Duration = 0.4f;
	}

	FFeelTrackClipboard& Clipboard = FFeelTrackClipboard::Get();
	const TArray<int32> TrackIndices = { 0, 1 };
	Clipboard.Copy(*Source, TrackIndices);
	TestEqual(TEXT("Both tracks copied"), Clipboard.Num(), 2);

	// Changes to the source after copying must not leak into the clipboard, and the copies must survive garbage collection.
	Flash->MaxOpacity = 0.1f;
	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);

	const int32 FirstPasted = Clipboard.PasteInto(*Target, 0, 1.0f);
	TestEqual(TEXT("Paste reports the first new index"), FirstPasted, 0);
	if (!TestEqual(TEXT("Target receives both tracks"), Target->Tracks.Num(), 2))
	{
		return false;
	}
	TestEqual(TEXT("Earliest pasted track starts at the paste time"), Target->Tracks[0].StartTime, 1.0f, 0.0001f);
	TestEqual(TEXT("Relative timing is kept"), Target->Tracks[1].StartTime, 1.3f, 0.0001f);

	const UFeelStep_ScreenFlash* PastedFlash = Cast<UFeelStep_ScreenFlash>(Target->Tracks[0].Step);
	if (!TestNotNull(TEXT("Pasted track has its step"), PastedFlash))
	{
		return false;
	}
	TestTrue(TEXT("Pasted step is a new object"), PastedFlash != Flash);
	TestTrue(TEXT("Pasted step belongs to the target recipe"), PastedFlash->GetOuter() == Target.Get());
	TestEqual(TEXT("Pasted step keeps the settings from copy time"), PastedFlash->MaxOpacity, 0.7f);
	TestTrue(TEXT("Pasted step supports undo"), PastedFlash->HasAnyFlags(RF_Transactional));

	Clipboard.PasteInto(*Target, 1, 0.0f);
	TestEqual(TEXT("Second paste inserts two more tracks"), Target->Tracks.Num(), 4);
	TestTrue(TEXT("Each paste creates its own steps"), Target->Tracks[1].Step != Target->Tracks[0].Step);
	TestEqual(TEXT("Second paste lands at its insert index"), Target->Tracks[1].StartTime, 0.0f, 0.0001f);

	Clipboard.Clear();
	TestEqual(TEXT("Nothing to paste after clearing"), Clipboard.PasteInto(*Target, 0, 0.0f), static_cast<int32>(INDEX_NONE));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelPreviewComfortTest, "FeelKit.Editor.PreviewComfort", FEEL_TEST_FLAGS)
bool FFeelPreviewComfortTest::RunTest(const FString& Parameters)
{
	TStrongObjectPtr<UFeelRecipe> Recipe(NewObject<UFeelRecipe>(GetTransientPackage()));
	UFeelStep_ScreenFlash* Flash = NewObject<UFeelStep_ScreenFlash>(Recipe.Get());
	Flash->MaxOpacity = 1.0f;
	{
		FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
		Track.Step = Flash;
		Track.Channel = FeelTags::Screen_Flash;
		Track.Duration = 1.0f;
		Track.IntensityCurve.GetRichCurve()->Reset();
	}

	const TSharedRef<FFeelRecipeEditorState> State = MakeShared<FFeelRecipeEditorState>(Recipe.Get());
	State->SetTime(0.5f);
	TestEqual(TEXT("Neutral preview shows the full flash"), State->GetOutput().FlashAlpha, 1.0f);

	State->SetPreviewComfortPreset(EFeelBuiltInComfortPreset::ReducedFlashing);
	const FFeelComfortScales& Reduced = GetDefault<UFeelSettings>()->ReducedFlashingPreset;
	TestEqual(TEXT("Reduced Flashing preview scales the flash"), State->GetOutput().FlashAlpha, Reduced.Flashes * Reduced.Master, 0.001f);
	TestTrue(TEXT("The preview remembers the preset"), State->GetPreviewComfortPreset() == TOptional<EFeelBuiltInComfortPreset>(EFeelBuiltInComfortPreset::ReducedFlashing));

	State->SetPreviewComfortPreset(TOptional<EFeelBuiltInComfortPreset>());
	TestEqual(TEXT("Back to neutral"), State->GetOutput().FlashAlpha, 1.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelCurveKeyEditingTest, "FeelKit.Editor.CurveKeyEditing", FEEL_TEST_FLAGS)
bool FFeelCurveKeyEditingTest::RunTest(const FString& Parameters)
{
	TStrongObjectPtr<UFeelRecipe> Recipe(NewObject<UFeelRecipe>(GetTransientPackage()));
	{
		FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
		Track.Step = NewObject<UFeelStep_ScreenFlash>(Recipe.Get());
		Track.Duration = 1.0f;
		Track.IntensityCurve.GetRichCurve()->Reset();
	}

	const TSharedRef<FFeelRecipeEditorState> State = MakeShared<FFeelRecipeEditorState>(Recipe.Get());
	const FRichCurve* Curve = State->GetEditableCurve(0);
	if (!TestNotNull(TEXT("The track's own curve is editable"), Curve))
	{
		return false;
	}
	TestNull(TEXT("An invalid track has no editable curve"), State->GetEditableCurve(3));

	TestTrue(TEXT("Key added outside the range"), State->AddCurveKey(0, -0.5f, 2.0f));
	TestTrue(TEXT("Middle key added"), State->AddCurveKey(0, 0.5f, 0.5f));
	TestTrue(TEXT("End key added"), State->AddCurveKey(0, 1.0f, 0.0f));
	if (!TestEqual(TEXT("Three keys"), Curve->GetNumKeys(), 3))
	{
		return false;
	}

	const FKeyHandle FirstKey = Curve->GetFirstKeyHandle();
	const FKeyHandle MiddleKey = Curve->GetNextKey(FirstKey);
	TestEqual(TEXT("Key time is clamped to 0"), Curve->GetKeyTime(FirstKey), 0.0f, 0.0001f);
	TestEqual(TEXT("Key value is clamped to 1"), Curve->GetKeyValue(FirstKey), 1.0f, 0.0001f);

	State->MoveCurveKey(0, MiddleKey, 2.0f, -1.0f);
	TestTrue(TEXT("Moved key stays before the next key"), Curve->GetKeyTime(MiddleKey) < 1.0f && Curve->GetKeyTime(MiddleKey) > 0.99f);
	TestEqual(TEXT("Moved key value is clamped to 0"), Curve->GetKeyValue(MiddleKey), 0.0f, 0.0001f);

	State->MoveCurveKey(0, MiddleKey, -1.0f, 0.25f);
	TestTrue(TEXT("Moved key stays after the previous key"), Curve->GetKeyTime(MiddleKey) > 0.0f && Curve->GetKeyTime(MiddleKey) < 0.01f);
	TestEqual(TEXT("Moved key takes the new value"), Curve->GetKeyValue(MiddleKey), 0.25f, 0.0001f);
	TestTrue(TEXT("Key order never changes while moving"), Curve->GetNextKey(FirstKey) == MiddleKey);

	TestTrue(TEXT("Interpolation changed"), State->SetCurveKeyInterpMode(0, MiddleKey, RCIM_Constant));
	TestTrue(TEXT("Key uses the new interpolation"), Curve->GetKeyInterpMode(MiddleKey) == RCIM_Constant);

	TestTrue(TEXT("Key deleted"), State->DeleteCurveKey(0, MiddleKey));
	TestEqual(TEXT("Two keys left"), Curve->GetNumKeys(), 2);
	TestFalse(TEXT("A deleted key cannot be deleted again"), State->DeleteCurveKey(0, MiddleKey));

	// A curve asset is edited in its own editor, never through the track.
	Recipe->Tracks[0].IntensityCurve.ExternalCurve = NewObject<UCurveFloat>(Recipe.Get());
	TestNull(TEXT("A track using a curve asset has no editable curve"), State->GetEditableCurve(0));
	TestFalse(TEXT("Keys cannot be added to a track using a curve asset"), State->AddCurveKey(0, 0.5f, 0.5f));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelPreviewParametersTest, "FeelKit.Editor.PreviewParameters", FEEL_TEST_FLAGS)
bool FFeelPreviewParametersTest::RunTest(const FString& Parameters)
{
	const FName DamageName(TEXT("Damage"));
	TStrongObjectPtr<UFeelRecipe> Recipe(NewObject<UFeelRecipe>(GetTransientPackage()));
	FFeelRecipeParameter& Damage = Recipe->Parameters.AddDefaulted_GetRef();
	Damage.Name = DamageName;
	Damage.MaxValue = 100.0f;
	Damage.DefaultValue = 20.0f;
	{
		FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
		UFeelStep_ScreenFlash* Flash = NewObject<UFeelStep_ScreenFlash>(Recipe.Get());
		Flash->MaxOpacity = 1.0f;
		Track.Step = Flash;
		Track.Channel = FeelTags::Screen_Flash;
		Track.Duration = 1.0f;
		Track.IntensityCurve.GetRichCurve()->Reset();
		Track.ParameterMappings.AddDefaulted_GetRef().Parameter = DamageName;
	}

	const TSharedRef<FFeelRecipeEditorState> State = MakeShared<FFeelRecipeEditorState>(Recipe.Get());
	TestEqual(TEXT("Slider starts at the default"), State->GetPreviewParameterValue(DamageName), 20.0f);

	State->SetTime(0.5f);
	TestEqual(TEXT("Preview plays with the default value"), State->GetOutput().FlashAlpha, 0.2f, 0.001f);

	State->SetPreviewParameterValue(DamageName, 80.0f);
	TestEqual(TEXT("Moving the slider updates the preview at once"), State->GetOutput().FlashAlpha, 0.8f, 0.001f);
	TestEqual(TEXT("The slider reads back its value"), State->GetPreviewParameterValue(DamageName), 80.0f);
	TestEqual(TEXT("The recipe default is not modified"), Recipe->Parameters[0].DefaultValue, 20.0f);

	State->ResetPreviewParameters();
	TestEqual(TEXT("Defaults button restores the default"), State->GetOutput().FlashAlpha, 0.2f, 0.001f);

	Recipe->Tracks[0].RandomDurationScale = FFloatInterval(3.0f, 3.0f);
	TestEqual(TEXT("Preview length includes random duration"), State->GetPlaybackLength(), 3.0f, 0.001f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelSaveValidationLogTest, "FeelKit.Editor.SaveValidationLog", FEEL_TEST_FLAGS)
bool FFeelSaveValidationLogTest::RunTest(const FString& Parameters)
{
	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>(TEXT("MessageLog"));
	if (!MessageLogModule.IsRegisteredLogListing(TEXT("AssetCheck")))
	{
		MessageLogModule.RegisterLogListing(TEXT("AssetCheck"), FText::FromString(TEXT("Asset Check")));
	}

	const FName RecipeName(TEXT("R_FeelKitTest_SaveLog"));
	const FName OtherName(TEXT("R_FeelKitTest_SaveLogOther"));
	TestFalse(TEXT("Nothing to clear before the recipe was ever validated"), FeelSaveValidationLog::ClearSavePage(TEXT("R_FeelKitTest_NeverSaved")));

	// An earlier save left an error on the recipe's page, and another asset has its own page.
	FMessageLog AssetCheck(TEXT("AssetCheck"));
	AssetCheck.SuppressLoggingToOutputLog(true);
	AssetCheck.SetCurrentPage(FeelSaveValidationLog::GetSavePageTitle(OtherName));
	AssetCheck.Error(FText::FromString(TEXT("Other asset problem")));
	AssetCheck.Flush();
	AssetCheck.SetCurrentPage(FeelSaveValidationLog::GetSavePageTitle(RecipeName));
	AssetCheck.Error(FText::FromString(TEXT("Old recipe problem")));
	AssetCheck.Flush();
	TestEqual(TEXT("Recipe page holds the old error"), AssetCheck.NumMessages(EMessageSeverity::Warning), 1);

	// A different asset is current when the recipe saves.
	AssetCheck.SetCurrentPage(FeelSaveValidationLog::GetSavePageTitle(OtherName));

	TestTrue(TEXT("Recipe page found and cleared"), FeelSaveValidationLog::ClearSavePage(RecipeName));
	AssetCheck.SetCurrentPage(FeelSaveValidationLog::GetSavePageTitle(RecipeName));
	TestEqual(TEXT("Old recipe messages are gone, so a clean save does not open the log"), AssetCheck.NumMessages(EMessageSeverity::Info), 0);

	AssetCheck.SetCurrentPage(FeelSaveValidationLog::GetSavePageTitle(OtherName));
	TestEqual(TEXT("Other assets keep their messages"), AssetCheck.NumMessages(EMessageSeverity::Warning), 1);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelPreviewReleaseParameterTest, "FeelKit.Editor.PreviewReleaseParameter", FEEL_TEST_FLAGS)
bool FFeelPreviewReleaseParameterTest::RunTest(const FString& Parameters)
{
	// The same recipe as FeelKit.Runtime.ReleaseParameter: loop 0.2 to 0.6 s, a full-charge ending and an early ending.
	const FName Charge(TEXT("Charge"));
	TStrongObjectPtr<UFeelRecipe> Recipe(NewObject<UFeelRecipe>(GetTransientPackage()));
	UFeelStep_ScreenFlash* Flash = NewObject<UFeelStep_ScreenFlash>(Recipe.Get());
	auto AddTrack = [&Recipe, Flash](float StartTime, float Duration, EFeelReleaseCondition Release)
	{
		FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
		Track.Step = Flash;
		Track.Channel = FeelTags::Screen_Flash;
		Track.StartTime = StartTime;
		Track.Duration = Duration;
		Track.IntensityCurve.GetRichCurve()->Reset();
		Track.Conditions.Release = Release;
	};
	AddTrack(0.0f, 1.0f, EFeelReleaseCondition::Any);
	AddTrack(0.6f, 0.5f, EFeelReleaseCondition::WhenReleaseParameterReached);
	AddTrack(0.6f, 0.5f, EFeelReleaseCondition::WhenReleasedEarly);
	FFeelRecipeParameter& Parameter = Recipe->Parameters.AddDefaulted_GetRef();
	Parameter.Name = Charge;
	Recipe->bSustain = true;
	Recipe->SustainStart = 0.2f;
	Recipe->SustainEnd = 0.6f;
	Recipe->bJumpToEndOnRelease = true;
	Recipe->ReleaseParameter = Charge;

	const TSharedRef<FFeelRecipeEditorState> State = MakeShared<FFeelRecipeEditorState>(Recipe.Get());

	// The slider reaching Release At releases the preview, which jumps into the ending.
	State->TogglePlay();
	for (int32 Frame = 0; Frame < 10; ++Frame)
	{
		State->Tick(0.1f);
	}
	TestTrue(TEXT("Below Release At the preview keeps looping"), State->CanReleaseSustain());
	TestTrue(TEXT("The full-charge track says what it waits for"), State->GetTrackDecision(1).ToString().Contains(TEXT("Charge reaches Release At")));
	State->SetPreviewParameterValue(Charge, 1.0f);
	State->Tick(0.1f);
	TestFalse(TEXT("The slider at Release At releases the preview"), State->CanReleaseSustain());
	TestTrue(TEXT("The preview jumps into the ending"), State->GetTime() >= 0.6f && State->GetTime() < 0.75f);
	TestTrue(TEXT("The full-charge track plays"), State->GetTrackDecision(1).IsEmpty());
	TestTrue(TEXT("The early-release track is skipped"), State->GetTrackDecision(2).ToString().Contains(TEXT("Skipped")));

	// The Release button before a full charge jumps at once and plays the early ending.
	State->Stop();
	State->ResetPreviewParameters();
	State->TogglePlay();
	for (int32 Frame = 0; Frame < 3; ++Frame)
	{
		State->Tick(0.1f);
	}
	State->ReleaseSustain();
	TestEqual(TEXT("The Release button jumps straight to Sustain End"), State->GetTime(), 0.6f, 0.0001f);
	State->Tick(0.1f);
	TestTrue(TEXT("The early-release track plays"), State->GetTrackDecision(2).IsEmpty());
	TestTrue(TEXT("The full-charge track is skipped"), State->GetTrackDecision(1).ToString().Contains(TEXT("released before Charge reached Release At")));

	return true;
}

#undef FEEL_TEST_FLAGS

#endif // WITH_DEV_AUTOMATION_TESTS
