// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "FeelEditorColors.h"
#include "FeelRecipe.h"
#include "FeelTags.h"
#include "FeelThumbnailLayout.h"
#include "Steps/FeelStep_ProceduralShake.h"
#include "Steps/FeelStep_ScreenFlash.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"

#define FEEL_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelThumbnailLayoutTest, "FeelKit.Editor.ThumbnailLayout", FEEL_TEST_FLAGS)
bool FFeelThumbnailLayoutTest::RunTest(const FString& Parameters)
{
	TStrongObjectPtr<UFeelRecipe> Recipe(NewObject<UFeelRecipe>(GetTransientPackage()));

	// A full-strength shake over the first half.
	{
		FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
		Track.Step = NewObject<UFeelStep_ProceduralShake>(Recipe.Get());
		Track.Channel = FeelTags::Camera_Shake;
		Track.StartTime = 0.0f;
		Track.Duration = 0.5f;
		Track.IntensityCurve.GetRichCurve()->Reset(); // An empty curve means constant full intensity.
	}

	// A flash that fades out, from a quarter of the way to the end.
	{
		FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
		Track.Step = NewObject<UFeelStep_ScreenFlash>(Recipe.Get());
		Track.Channel = FeelTags::Screen_Flash;
		Track.StartTime = 0.25f;
		Track.Duration = 0.75f;
		FRichCurve* Curve = Track.IntensityCurve.GetRichCurve();
		Curve->Reset();
		Curve->AddKey(0.0f, 1.0f);
		Curve->AddKey(1.0f, 0.0f);
	}

	// A muted track is drawn faded.
	{
		FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
		Track.Step = NewObject<UFeelStep_ScreenFlash>(Recipe.Get());
		Track.Channel = FeelTags::Screen_Flash;
		Track.StartTime = 0.0f;
		Track.Duration = 0.2f;
		Track.bEnabled = false;
	}

	FFeelThumbnailLayout Layout = FeelThumbnailLayout::Build(*Recipe);
	TestEqual(TEXT("Length is the recipe duration"), Layout.Length, 1.0f, 0.0001f);
	if (!TestEqual(TEXT("One bar per track"), Layout.Bars.Num(), 3))
	{
		return false;
	}
	TestEqual(TEXT("Bar 0 spans the first half"), Layout.Bars[0].StartX, 0.0f, 0.0001f);
	TestEqual(TEXT("Bar 0 ends at the half"), Layout.Bars[0].EndX, 0.5f, 0.0001f);
	TestEqual(TEXT("Bar 1 starts a quarter in"), Layout.Bars[1].StartX, 0.25f, 0.0001f);
	TestEqual(TEXT("Bar 1 ends at the end"), Layout.Bars[1].EndX, 1.0f, 0.0001f);
	TestTrue(TEXT("Bar colors follow the channel"), Layout.Bars[0].Color.Equals(FeelEditorColors::GetChannelColor(FeelTags::Camera_Shake)));

	TestTrue(TEXT("A track without a curve is drawn at full height"), Layout.Bars[0].Heights.Num() > 1 && Layout.Bars[0].Heights[0] == 1.0f && Layout.Bars[0].Heights.Last() == 1.0f);
	TestTrue(TEXT("A fading track is drawn fading"), Layout.Bars[1].Heights[0] == 1.0f && Layout.Bars[1].Heights.Last() < 0.01f);
	TestFalse(TEXT("Playing tracks are not faded"), Layout.Bars[0].bFaded);
	TestTrue(TEXT("Muted tracks are faded"), Layout.Bars[2].bFaded);
	TestEqual(TEXT("Nothing is hidden yet"), Layout.HiddenTrackCount, 0);
	TestFalse(TEXT("No sustain region"), Layout.bHasSustain);
	TestTrue(TEXT("A recipe without a feeling is gray"), Layout.FeelingColor.Equals(FeelEditorColors::GetFeelingColor(FGameplayTag())));

	// Instant tracks still get a visible width.
	{
		FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
		Track.Step = NewObject<UFeelStep_ScreenFlash>(Recipe.Get());
		Track.StartTime = 0.5f;
		Track.Duration = 0.0f;
	}
	Layout = FeelThumbnailLayout::Build(*Recipe);
	TestTrue(TEXT("An instant track is still visible"), Layout.Bars[3].EndX > Layout.Bars[3].StartX);

	// More tracks than fit are counted.
	for (int32 Index = 0; Index < 6; ++Index)
	{
		FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
		Track.Step = NewObject<UFeelStep_ScreenFlash>(Recipe.Get());
		Track.Duration = 0.1f;
	}
	Layout = FeelThumbnailLayout::Build(*Recipe);
	TestEqual(TEXT("At most eight bars are drawn"), Layout.Bars.Num(), 8);
	TestEqual(TEXT("The rest are counted"), Layout.HiddenTrackCount, 2);

	// Sustain region.
	Recipe->bSustain = true;
	Recipe->SustainStart = 0.2f;
	Recipe->SustainEnd = 0.6f;
#if WITH_EDITORONLY_DATA
	Recipe->Feeling = FeelTags::Feeling_Impact;
#endif
	Layout = FeelThumbnailLayout::Build(*Recipe);
	TestTrue(TEXT("Sustain is reported"), Layout.bHasSustain);
	TestEqual(TEXT("Sustain starts at a fifth"), Layout.SustainStartX, 0.2f, 0.0001f);
	TestEqual(TEXT("Sustain ends at three fifths"), Layout.SustainEndX, 0.6f, 0.0001f);
#if WITH_EDITORONLY_DATA
	TestTrue(TEXT("The feeling color is used"), Layout.FeelingColor.Equals(FeelEditorColors::GetFeelingColor(FeelTags::Feeling_Impact)));
#endif
	TestFalse(TEXT("A transient recipe is not a library recipe"), Layout.bLibrary);

	return true;
}

#endif
