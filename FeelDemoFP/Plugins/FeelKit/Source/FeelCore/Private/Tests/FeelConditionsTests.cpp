// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "FeelComfortTypes.h"
#include "FeelEvaluator.h"
#include "FeelFrameOutput.h"
#include "FeelRecipe.h"
#include "FeelTags.h"
#include "HAL/PlatformProperties.h"
#include "Steps/FeelStep_ProceduralShake.h"
#include "Steps/FeelStep_ScreenFlash.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#define FEEL_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

namespace FeelConditionsTests
{
	/** Recipe with one constant-intensity flash on Feel.Screen.Flash from 0 to 1 s. */
	UFeelRecipe* MakeFlashRecipe()
	{
		UFeelRecipe* Recipe = NewObject<UFeelRecipe>(GetTransientPackage());
		FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
		UFeelStep_ScreenFlash* Flash = NewObject<UFeelStep_ScreenFlash>(Recipe);
		Flash->MaxOpacity = 1.0f;
		Track.Step = Flash;
		Track.Duration = 1.0f;
		Track.Channel = FeelTags::Screen_Flash;
		Track.IntensityCurve.GetRichCurve()->Reset();
		return Recipe;
	}

	int32 CountEvaluated(const UFeelRecipe& Recipe, const FFeelEvalParams& Params)
	{
		FFeelOutputAccumulator Accumulator;
		return FFeelEvaluator::Evaluate(Recipe, 0.5f, Params, Accumulator);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelComfortMotionOffTest, "FeelKit.Comfort.MotionOffMeansOff", FEEL_TEST_FLAGS)
bool FFeelComfortMotionOffTest::RunTest(const FString& Parameters)
{
	TStrongObjectPtr<UFeelRecipe> Recipe(NewObject<UFeelRecipe>(GetTransientPackage()));
	FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
	Track.Step = NewObject<UFeelStep_ProceduralShake>(Recipe.Get());
	Track.Channel = FeelTags::Camera_Shake;
	Track.Duration = 1.0f;
	Track.bEssential = true;
	Track.EssentialFloor = 0.5f;

	const TArray<FFeelChannelComfortMapping> Mappings = {
		FFeelChannelComfortMapping(FeelTags::Camera_Shake, EFeelComfortGroup::CameraShake),
		FFeelChannelComfortMapping(FeelTags::Camera_Motion, EFeelComfortGroup::CameraMotion),
	};
	FFeelComfortScales Scales;
	FFeelComfortContext Comfort;
	Comfort.Scales = &Scales;
	Comfort.Mappings = Mappings;

	TestTrue(TEXT("Camera Shake is a motion group"), FeelComfort::IsMotionGroup(EFeelComfortGroup::CameraShake));
	TestTrue(TEXT("Camera Motion is a motion group"), FeelComfort::IsMotionGroup(EFeelComfortGroup::CameraMotion));
	TestFalse(TEXT("Flashes are not a motion group"), FeelComfort::IsMotionGroup(EFeelComfortGroup::Flashes));

	float ComfortScale = -1.0f;
	Scales.CameraShake = 0.0f;
	TestNull(TEXT("Shake off: an essential shake without a substitute does not play, whatever its floor"), FFeelEvaluator::ResolveStep(Track, Comfort, ComfortScale));

	FFeelEvalParams Params;
	Params.Comfort = Comfort;
	TestEqual(TEXT("Shake off: nothing is evaluated"), FeelConditionsTests::CountEvaluated(*Recipe, Params), 0);

	Scales.CameraShake = 0.1f;
	TestNotNull(TEXT("Shake reduced: the track plays"), FFeelEvaluator::ResolveStep(Track, Comfort, ComfortScale));
	TestEqual(TEXT("Shake reduced: the player's scale is used, not raised to the floor"), ComfortScale, 0.1f, 0.0001f);

	Scales.CameraShake = 1.0f;
	Scales.Master = 0.0f;
	TestNull(TEXT("Master off: motion does not play either"), FFeelEvaluator::ResolveStep(Track, Comfort, ComfortScale));

	// A substitute on a non-motion channel still replaces the shake.
	Scales.Master = 1.0f;
	Scales.CameraShake = 0.0f;
	Track.SubstituteStep = NewObject<UFeelStep_ScreenFlash>(Recipe.Get());
	TestTrue(TEXT("Shake off: a flash substitute plays instead"), FFeelEvaluator::ResolveStep(Track, Comfort, ComfortScale) == Track.SubstituteStep);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelConditionsChanceTest, "FeelKit.Conditions.Chance", FEEL_TEST_FLAGS)
bool FFeelConditionsChanceTest::RunTest(const FString& Parameters)
{
	using namespace FeelConditionsTests;

	TStrongObjectPtr<UFeelRecipe> Recipe(MakeFlashRecipe());
	FFeelTrack& Track = Recipe->Tracks[0];
	FFeelEvalParams Params;

	Track.Conditions.Chance = 1.0f;
	TestEqual(TEXT("Chance 1 always plays"), CountEvaluated(*Recipe, Params), 1);
	Track.Conditions.Chance = 0.0f;
	TestEqual(TEXT("Chance 0 never plays"), CountEvaluated(*Recipe, Params), 0);

	Track.Conditions.Chance = 0.5f;
	constexpr int32 NumPlays = 2000;
	int32 Played = 0;
	int32 Unstable = 0;
	for (int32 Seed = 0; Seed < NumPlays; ++Seed)
	{
		Params.InstanceSeed = Seed * 7919 + 13;
		const bool bFirst = FFeelEvaluator::PassesConditions(Track, 0, Params);
		const bool bSecond = FFeelEvaluator::PassesConditions(Track, 0, Params);
		Played += bFirst ? 1 : 0;
		Unstable += bFirst != bSecond ? 1 : 0;
	}
	TestEqual(TEXT("One play always gives the same answer (scrub-safe)"), Unstable, 0);
	TestTrue(FString::Printf(TEXT("About half of the plays pass at chance 0.5 (got %d of %d)"), Played, NumPlays), Played > NumPlays * 0.44f && Played < NumPlays * 0.56f);

	// Two tracks with the same settings roll independently.
	int32 Different = 0;
	for (int32 Seed = 0; Seed < 200; ++Seed)
	{
		Different += (FFeelEvaluator::GetChanceRoll(Track, 0, Seed) < 0.5f) != (FFeelEvaluator::GetChanceRoll(Track, 1, Seed) < 0.5f) ? 1 : 0;
	}
	TestTrue(TEXT("Tracks with the same seed do not share their roll"), Different > 50);

	for (int32 Seed = 0; Seed < 200; ++Seed)
	{
		const float Roll = FFeelEvaluator::GetChanceRoll(Track, 3, Seed);
		if (Roll < 0.0f || Roll >= 1.0f)
		{
			AddError(FString::Printf(TEXT("Roll %f is outside [0, 1)"), Roll));
			break;
		}
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelConditionsTargetTest, "FeelKit.Conditions.DistanceLocalPlayerPlatform", FEEL_TEST_FLAGS)
bool FFeelConditionsTargetTest::RunTest(const FString& Parameters)
{
	using namespace FeelConditionsTests;

	TStrongObjectPtr<UFeelRecipe> Recipe(MakeFlashRecipe());
	FFeelTrack& Track = Recipe->Tracks[0];
	FFeelEvalParams Params;

	Track.Conditions.MaxDistance = 1000.0f;
	Params.TargetDistance = 500.0f;
	TestEqual(TEXT("Within max distance plays"), CountEvaluated(*Recipe, Params), 1);
	Params.TargetDistance = 1500.0f;
	TestEqual(TEXT("Beyond max distance is skipped"), CountEvaluated(*Recipe, Params), 0);
	Params.TargetDistance = -1.0f;
	TestEqual(TEXT("Unknown distance (no local camera) plays"), CountEvaluated(*Recipe, Params), 1);
	Track.Conditions.MaxDistance = 0.0f;
	Params.TargetDistance = 100000.0f;
	TestEqual(TEXT("Max distance 0 means unlimited"), CountEvaluated(*Recipe, Params), 1);

	Track.Conditions.bLocalPlayerOnly = true;
	Params.bTargetIsLocalPlayer = false;
	TestEqual(TEXT("Local player only skips other targets"), CountEvaluated(*Recipe, Params), 0);
	Params.bTargetIsLocalPlayer = true;
	TestEqual(TEXT("Local player only plays on a local player's target"), CountEvaluated(*Recipe, Params), 1);
	Track.Conditions.bLocalPlayerOnly = false;

	Track.Conditions.Platforms = { FName(TEXT("NotARealPlatform")) };
	TestEqual(TEXT("A platform filter without this platform skips the track"), CountEvaluated(*Recipe, Params), 0);
	Track.Conditions.Platforms.Add(FName(FPlatformProperties::IniPlatformName()));
	TestEqual(TEXT("A platform filter with this platform plays the track"), CountEvaluated(*Recipe, Params), 1);

	return true;
}

#if WITH_EDITOR
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelConditionsValidationTest, "FeelKit.Validation.ComfortAndConditions", FEEL_TEST_FLAGS)
bool FFeelConditionsValidationTest::RunTest(const FString& Parameters)
{
	auto CountWarnings = [](const UFeelRecipe& Recipe)
	{
		FDataValidationContext Context;
		Recipe.IsDataValid(Context);
		return Context.GetNumWarnings();
	};

	TStrongObjectPtr<UFeelRecipe> Recipe(NewObject<UFeelRecipe>(GetTransientPackage()));
	FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
	Track.Step = NewObject<UFeelStep_ProceduralShake>(Recipe.Get());
	Track.Channel = FeelTags::Camera_Shake;
	Track.Duration = 1.0f;
	TestEqual(TEXT("A plain shake track has no warnings"), CountWarnings(*Recipe), 0);

	Track.bEssential = true;
	Track.EssentialFloor = 0.5f;
	TestEqual(TEXT("Essential shake relying on a floor warns that the floor is ignored"), CountWarnings(*Recipe), 1);

	Track.SubstituteStep = NewObject<UFeelStep_ProceduralShake>(Recipe.Get());
	TestEqual(TEXT("A motion substitute for motion warns"), CountWarnings(*Recipe), 1);

	Track.SubstituteStep = NewObject<UFeelStep_ScreenFlash>(Recipe.Get());
	TestEqual(TEXT("A flash substitute for a shake is fine"), CountWarnings(*Recipe), 0);

	Track.Conditions.Chance = 0.0f;
	TestEqual(TEXT("Chance 0 warns that the track never plays"), CountWarnings(*Recipe), 1);

	return true;
}
#endif

#undef FEEL_TEST_FLAGS

#endif // WITH_DEV_AUTOMATION_TESTS
