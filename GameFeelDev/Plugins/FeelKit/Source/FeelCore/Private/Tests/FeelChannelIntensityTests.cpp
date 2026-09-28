// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "FeelComfortTypes.h"
#include "FeelEvaluator.h"
#include "FeelRecipe.h"
#include "FeelTags.h"
#include "Steps/FeelStep_ProceduralShake.h"
#include "Steps/FeelStep_ScreenFlash.h"
#include "UObject/Package.h"

#define FEEL_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

namespace FeelChannelIntensityTests
{
	/** Adds a track whose intensity curve is a constant Value. */
	void AddTrack(UFeelRecipe* Recipe, UFeelStep* Step, const FGameplayTag& Channel, float StartTime, float Duration, float Value)
	{
		FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
		Track.Step = Step;
		Track.Channel = Channel;
		Track.StartTime = StartTime;
		Track.Duration = Duration;
		FRichCurve* Curve = Track.IntensityCurve.GetRichCurve();
		Curve->Reset();
		Curve->AddKey(0.0f, Value);
	}

	void TestSamples(FAutomationTestBase& Test, const TCHAR* What, const TArray<float>& Actual, const TArray<float>& Expected)
	{
		if (!Test.TestEqual(FString::Printf(TEXT("%s: sample count"), What), Actual.Num(), Expected.Num()))
		{
			return;
		}
		for (int32 Index = 0; Index < Expected.Num(); ++Index)
		{
			Test.TestEqual(FString::Printf(TEXT("%s: sample %d"), What, Index), Actual[Index], Expected[Index], 0.001f);
		}
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelChannelIntensityTest, "FeelKit.Evaluator.ChannelIntensitySampling", FEEL_TEST_FLAGS)
bool FFeelChannelIntensityTest::RunTest(const FString& Parameters)
{
	using namespace FeelChannelIntensityTests;

	UFeelRecipe* Recipe = NewObject<UFeelRecipe>(GetTransientPackage());
	AddTrack(Recipe, NewObject<UFeelStep_ScreenFlash>(Recipe), FeelTags::Screen_Flash, 0.0f, 1.0f, 0.5f);
	AddTrack(Recipe, NewObject<UFeelStep_ScreenFlash>(Recipe), FeelTags::Screen_Flash, 0.5f, 0.5f, 1.0f);
	AddTrack(Recipe, NewObject<UFeelStep_ProceduralShake>(Recipe), FeelTags::Camera_Shake, 0.0f, 0.5f, 1.0f);
	AddTrack(Recipe, NewObject<UFeelStep_ScreenFlash>(Recipe), FeelTags::Screen_Flash, 0.0f, 1.0f, 1.0f);
	Recipe->Tracks[3].bEnabled = false;

	TArray<FFeelChannelIntensity> Channels;
	FFeelEvaluator::SampleChannelIntensities(*Recipe, 5, FFeelEvalParams(), Channels);
	if (!TestEqual(TEXT("One entry per channel"), Channels.Num(), 2))
	{
		return false;
	}

	TestTrue(TEXT("Channels are sorted by tag name"), Channels[0].Channel == FeelTags::Camera_Shake.GetTag() && Channels[1].Channel == FeelTags::Screen_Flash.GetTag());
	TestSamples(*this, TEXT("Shake over time"), Channels[0].Samples, { 1.0f, 1.0f, 1.0f, 0.0f, 0.0f });
	TestSamples(*this, TEXT("Flash keeps the strongest overlapping track; muted track ignored"), Channels[1].Samples, { 0.5f, 0.5f, 1.0f, 1.0f, 1.0f });

	const TArray<FFeelChannelComfortMapping> Mappings = { FFeelChannelComfortMapping(FeelTags::Screen_Flash, EFeelComfortGroup::Flashes) };
	FFeelComfortScales Scales;
	Scales.Flashes = 0.5f;
	FFeelEvalParams ComfortParams;
	ComfortParams.Comfort.Scales = &Scales;
	ComfortParams.Comfort.Mappings = Mappings;

	FFeelEvaluator::SampleChannelIntensities(*Recipe, 5, ComfortParams, Channels);
	TestSamples(*this, TEXT("Comfort scales the flash channel"), Channels[1].Samples, { 0.25f, 0.25f, 0.5f, 0.5f, 0.5f });
	TestSamples(*this, TEXT("Unmapped shake channel stays at full strength"), Channels[0].Samples, { 1.0f, 1.0f, 1.0f, 0.0f, 0.0f });

	return true;
}

#undef FEEL_TEST_FLAGS

#endif // WITH_DEV_AUTOMATION_TESTS
