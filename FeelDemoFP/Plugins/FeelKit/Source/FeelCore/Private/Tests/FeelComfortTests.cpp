// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "FeelComfortSaveGame.h"
#include "FeelComfortStorage.h"
#include "FeelComfortTypes.h"
#include "FeelEvaluator.h"
#include "FeelFrameOutput.h"
#include "FeelRecipe.h"
#include "FeelSettings.h"
#include "FeelTags.h"
#include "Kismet/GameplayStatics.h"
#include "Steps/FeelStep_ScalePunch.h"
#include "Steps/FeelStep_ScreenFlash.h"
#include "Tests/FeelComfortTestStorage.h"
#include "UObject/Package.h"

#define FEEL_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

namespace FeelComfortTests
{
	/** Recipe with one full-opacity flash on Feel.Screen.Flash from 0 to 2 s, with a constant curve. */
	UFeelRecipe* MakeFlashRecipe()
	{
		UFeelRecipe* Recipe = NewObject<UFeelRecipe>(GetTransientPackage());
		FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
		UFeelStep_ScreenFlash* Flash = NewObject<UFeelStep_ScreenFlash>(Recipe);
		Flash->MaxOpacity = 1.0f;
		Track.Step = Flash;
		Track.Duration = 2.0f;
		Track.Channel = FeelTags::Screen_Flash;
		Track.IntensityCurve.GetRichCurve()->Reset();
		return Recipe;
	}

	TArray<FFeelChannelComfortMapping> MakeMappings()
	{
		return {
			FFeelChannelComfortMapping(FeelTags::Screen_Flash, EFeelComfortGroup::Flashes),
			FFeelChannelComfortMapping(FeelTags::Camera_Shake, EFeelComfortGroup::CameraShake),
		};
	}

	FFeelEvalParams MakeParams(const FFeelComfortScales& Scales, const TArray<FFeelChannelComfortMapping>& Mappings)
	{
		FFeelEvalParams Params;
		Params.Comfort.Scales = &Scales;
		Params.Comfort.Mappings = Mappings;
		return Params;
	}

	FFeelFrameOutput EvaluateAt(const UFeelRecipe* Recipe, float Time, const FFeelEvalParams& Params, int32* OutEvaluatedTracks = nullptr)
	{
		FFeelOutputAccumulator Accumulator;
		const int32 EvaluatedTracks = FFeelEvaluator::Evaluate(*Recipe, Time, Params, Accumulator);
		if (OutEvaluatedTracks)
		{
			*OutEvaluatedTracks = EvaluatedTracks;
		}
		return Accumulator.Output;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelComfortIntensityTest, "FeelKit.Comfort.IntensityMath", FEEL_TEST_FLAGS)
bool FFeelComfortIntensityTest::RunTest(const FString& Parameters)
{
	using namespace FeelComfortTests;

	UFeelRecipe* Recipe = MakeFlashRecipe();
	const TArray<FFeelChannelComfortMapping> Mappings = MakeMappings();
	FFeelComfortScales Scales;
	Scales.Master = 0.8f;
	Scales.Flashes = 0.5f;
	FFeelEvalParams Params = MakeParams(Scales, Mappings);
	Params.Intensity = 0.5f;

	TestEqual(TEXT("Call x Channel x Master"), EvaluateAt(Recipe, 1.0f, Params).FlashAlpha, 0.2f);

	FRichCurve* Curve = Recipe->Tracks[0].IntensityCurve.GetRichCurve();
	Curve->SetKeyInterpMode(Curve->AddKey(0.0f, 1.0f), RCIM_Linear);
	Curve->SetKeyInterpMode(Curve->AddKey(1.0f, 0.0f), RCIM_Linear);
	TestEqual(TEXT("Call x Curve x Channel x Master"), EvaluateAt(Recipe, 1.0f, Params).FlashAlpha, 0.1f);

	Scales.CameraShake = 0.0f;
	TestEqual(TEXT("Other groups do not affect this channel"), EvaluateAt(Recipe, 1.0f, Params).FlashAlpha, 0.1f);

	FFeelEvalParams Neutral;
	Neutral.Intensity = 0.5f;
	TestEqual(TEXT("Without comfort, evaluation is neutral"), EvaluateAt(Recipe, 1.0f, Neutral).FlashAlpha, 0.25f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelComfortSkipTest, "FeelKit.Comfort.NonEssentialSkipped", FEEL_TEST_FLAGS)
bool FFeelComfortSkipTest::RunTest(const FString& Parameters)
{
	using namespace FeelComfortTests;

	UFeelRecipe* Recipe = MakeFlashRecipe();
	const TArray<FFeelChannelComfortMapping> Mappings = MakeMappings();
	FFeelComfortScales Scales;
	const FFeelEvalParams Params = MakeParams(Scales, Mappings);

	int32 EvaluatedTracks = -1;
	Scales.Flashes = 0.0f;
	TestEqual(TEXT("Channel at 0 produces no flash"), EvaluateAt(Recipe, 1.0f, Params, &EvaluatedTracks).FlashAlpha, 0.0f);
	TestEqual(TEXT("Channel at 0 skips the step entirely"), EvaluatedTracks, 0);

	Scales.Flashes = 1.0f;
	Scales.Master = 0.0f;
	TestEqual(TEXT("Master at 0 produces no flash"), EvaluateAt(Recipe, 1.0f, Params, &EvaluatedTracks).FlashAlpha, 0.0f);
	TestEqual(TEXT("Master at 0 skips the step entirely"), EvaluatedTracks, 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelComfortFloorTest, "FeelKit.Comfort.EssentialFloor", FEEL_TEST_FLAGS)
bool FFeelComfortFloorTest::RunTest(const FString& Parameters)
{
	using namespace FeelComfortTests;

	UFeelRecipe* Recipe = MakeFlashRecipe();
	Recipe->Tracks[0].bEssential = true;
	Recipe->Tracks[0].EssentialFloor = 0.3f;
	const TArray<FFeelChannelComfortMapping> Mappings = MakeMappings();
	FFeelComfortScales Scales;
	const FFeelEvalParams Params = MakeParams(Scales, Mappings);

	Scales.Flashes = 0.0f;
	TestEqual(TEXT("Channel at 0 keeps the floor"), EvaluateAt(Recipe, 1.0f, Params).FlashAlpha, 0.3f);

	Scales.Flashes = 0.1f;
	TestEqual(TEXT("Channel below the floor is raised to it"), EvaluateAt(Recipe, 1.0f, Params).FlashAlpha, 0.3f);

	Scales.Flashes = 0.6f;
	TestEqual(TEXT("Channel above the floor is used as is"), EvaluateAt(Recipe, 1.0f, Params).FlashAlpha, 0.6f);

	Scales.Flashes = 1.0f;
	Scales.Master = 0.0f;
	TestEqual(TEXT("Master at 0 still keeps the floor"), EvaluateAt(Recipe, 1.0f, Params).FlashAlpha, 0.3f);

	Recipe->Tracks[0].EssentialFloor = 0.0f;
	int32 EvaluatedTracks = -1;
	EvaluateAt(Recipe, 1.0f, Params, &EvaluatedTracks);
	TestEqual(TEXT("Essential track with floor 0 is skipped at scale 0"), EvaluatedTracks, 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelComfortSubstitutionTest, "FeelKit.Comfort.Substitution", FEEL_TEST_FLAGS)
bool FFeelComfortSubstitutionTest::RunTest(const FString& Parameters)
{
	using namespace FeelComfortTests;

	UFeelRecipe* Recipe = MakeFlashRecipe();
	FFeelTrack& Track = Recipe->Tracks[0];
	Track.bEssential = true;
	UFeelStep_ScalePunch* Punch = NewObject<UFeelStep_ScalePunch>(Recipe);
	Punch->Amount = FVector(0.5);
	Punch->Bounces = 0;
	Track.SubstituteStep = Punch;

	const TArray<FFeelChannelComfortMapping> Mappings = MakeMappings();
	FFeelComfortScales Scales;
	const FFeelEvalParams Params = MakeParams(Scales, Mappings);

	// Track alpha 0.5: a single-swing punch is at its peak.
	Scales.Flashes = 0.0f;
	FFeelFrameOutput Output = EvaluateAt(Recipe, 1.0f, Params);
	TestEqual(TEXT("The original flash does not play"), Output.FlashAlpha, 0.0f);
	TestTrue(TEXT("The substitute plays instead"), Output.TargetScaleDelta.Equals(FVector(0.5), 0.001));

	float ComfortScale = 0.0f;
	TestTrue(TEXT("ResolveStep picks the substitute"), FFeelEvaluator::ResolveStep(Track, Params.Comfort, ComfortScale) == Punch);

	Scales.Master = 0.5f;
	Output = EvaluateAt(Recipe, 1.0f, Params);
	TestTrue(TEXT("The substitute is scaled by Master and its own channel"), Output.TargetScaleDelta.Equals(FVector(0.25), 0.001));

	Scales.Master = 1.0f;
	Scales.Flashes = 1.0f;
	Output = EvaluateAt(Recipe, 1.0f, Params);
	TestEqual(TEXT("With the channel on, the original plays"), Output.FlashAlpha, 1.0f);
	TestTrue(TEXT("With the channel on, the substitute does not play"), Output.TargetScaleDelta.IsZero());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelComfortMappingTest, "FeelKit.Comfort.ChannelMapping", FEEL_TEST_FLAGS)
bool FFeelComfortMappingTest::RunTest(const FString& Parameters)
{
	const FGameplayTag CameraParent = FGameplayTag::RequestGameplayTag(TEXT("Feel.Camera"), false);
	if (!TestTrue(TEXT("Parent tag Feel.Camera exists"), CameraParent.IsValid()))
	{
		return false;
	}

	const TArray<FFeelChannelComfortMapping> ParentFirst = {
		FFeelChannelComfortMapping(CameraParent, EFeelComfortGroup::CameraMotion),
		FFeelChannelComfortMapping(FeelTags::Camera_Shake, EFeelComfortGroup::CameraShake),
	};
	const TArray<FFeelChannelComfortMapping> ChildFirst = { ParentFirst[1], ParentFirst[0] };

	TestTrue(TEXT("Most specific mapping wins when the parent is listed first"), FeelComfort::FindGroup(FeelTags::Camera_Shake, ParentFirst) == EFeelComfortGroup::CameraShake);
	TestTrue(TEXT("Most specific mapping wins when the child is listed first"), FeelComfort::FindGroup(FeelTags::Camera_Shake, ChildFirst) == EFeelComfortGroup::CameraShake);
	TestTrue(TEXT("A parent mapping covers other child channels"), FeelComfort::FindGroup(FeelTags::Camera_Motion, ParentFirst) == EFeelComfortGroup::CameraMotion);
	TestTrue(TEXT("Unmapped channels have no group"), FeelComfort::FindGroup(FeelTags::Screen_Flash, ParentFirst) == EFeelComfortGroup::None);
	TestTrue(TEXT("Invalid channels have no group"), FeelComfort::FindGroup(FGameplayTag(), ParentFirst) == EFeelComfortGroup::None);

	const UFeelSettings* Settings = GetDefault<UFeelSettings>();
	TestTrue(TEXT("Default mapping: flashes"), FeelComfort::FindGroup(FeelTags::Screen_Flash, Settings->ChannelComfortGroups) == EFeelComfortGroup::Flashes);
	TestTrue(TEXT("Default mapping: camera shake"), FeelComfort::FindGroup(FeelTags::Camera_Shake, Settings->ChannelComfortGroups) == EFeelComfortGroup::CameraShake);
	TestTrue(TEXT("Default mapping: hitstop"), FeelComfort::FindGroup(FeelTags::Time_Hitstop, Settings->ChannelComfortGroups) == EFeelComfortGroup::HitstopAndSlowMo);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelComfortPresetTest, "FeelKit.Comfort.Presets", FEEL_TEST_FLAGS)
bool FFeelComfortPresetTest::RunTest(const FString& Parameters)
{
	const UFeelSettings* Settings = GetDefault<UFeelSettings>();
	TestTrue(TEXT("Default preset is the project defaults"), Settings->GetPresetScales(EFeelBuiltInComfortPreset::Default) == Settings->DefaultComfortScales);
	TestTrue(TEXT("Reduced Motion preset comes from settings"), Settings->GetPresetScales(EFeelBuiltInComfortPreset::ReducedMotion) == Settings->ReducedMotionPreset);
	TestTrue(TEXT("Reduced Flashing preset comes from settings"), Settings->GetPresetScales(EFeelBuiltInComfortPreset::ReducedFlashing) == Settings->ReducedFlashingPreset);
	TestTrue(TEXT("No Haptics preset comes from settings"), Settings->GetPresetScales(EFeelBuiltInComfortPreset::NoHaptics) == Settings->NoHapticsPreset);

	TestTrue(TEXT("Reduced Motion reduces camera shake"), Settings->ReducedMotionPreset.CameraShake < 1.0f);
	TestTrue(TEXT("Reduced Flashing reduces flashes"), Settings->ReducedFlashingPreset.Flashes < 1.0f);
	TestEqual(TEXT("No Haptics turns haptics off"), Settings->NoHapticsPreset.Haptics, 0.0f);

	FFeelComfortScales Scales;
	Scales.Master = 2.0f;
	Scales.Flashes = -1.0f;
	Scales.ClampScales();
	TestEqual(TEXT("Scales clamp to 1"), Scales.Master, 1.0f);
	TestEqual(TEXT("Scales clamp to 0"), Scales.Flashes, 0.0f);

	Scales.SetGroupScale(EFeelComfortGroup::CameraShake, 0.4f);
	TestEqual(TEXT("Group scale can be set"), Scales.GetGroupScale(EFeelComfortGroup::CameraShake), 0.4f);
	TestEqual(TEXT("The None group always scales by 1"), Scales.GetGroupScale(EFeelComfortGroup::None), 1.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelComfortSaveGameTest, "FeelKit.Comfort.SaveGameRoundTrip", FEEL_TEST_FLAGS)
bool FFeelComfortSaveGameTest::RunTest(const FString& Parameters)
{
	const FString SlotName = TEXT("FeelKitAutomationTest_Comfort");
	UGameplayStatics::DeleteGameInSlot(SlotName, 0);

	FFeelComfortScales Loaded;
	TestFalse(TEXT("Loading a missing slot fails"), UFeelSaveGameComfortStorage::LoadFromSlot(SlotName, 0, Loaded));

	FFeelComfortScales Saved;
	Saved.Master = 0.7f;
	Saved.CameraShake = 0.1f;
	Saved.Haptics = 0.0f;
	TestTrue(TEXT("Saving succeeds"), UFeelSaveGameComfortStorage::SaveToSlot(SlotName, 0, Saved));
	TestTrue(TEXT("Loading succeeds"), UFeelSaveGameComfortStorage::LoadFromSlot(SlotName, 0, Loaded));
	TestTrue(TEXT("Loaded scales match the saved scales"), Loaded == Saved);

	UGameplayStatics::DeleteGameInSlot(SlotName, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelComfortStorageTest, "FeelKit.Comfort.CustomStorage", FEEL_TEST_FLAGS)
bool FFeelComfortStorageTest::RunTest(const FString& Parameters)
{
	UFeelComfortTestStorage* Storage = NewObject<UFeelComfortTestStorage>(GetTransientPackage());

	FFeelComfortScales Loaded;
	TestFalse(TEXT("Nothing stored yet"), FeelComfort::LoadScales(Storage, nullptr, Loaded));

	FFeelComfortScales Saved;
	Saved.Flashes = 0.25f;
	TestTrue(TEXT("Saving routes to the custom storage"), FeelComfort::SaveScales(Storage, nullptr, Saved));
	TestEqual(TEXT("The custom storage received one save"), Storage->SaveCount, 1);
	TestTrue(TEXT("Loading routes to the custom storage"), FeelComfort::LoadScales(Storage, nullptr, Loaded));
	TestTrue(TEXT("Round trip through the custom storage"), Loaded == Saved);

	UObject* NotStorage = NewObject<UFeelRecipe>(GetTransientPackage());
	TestFalse(TEXT("Objects without the storage interface are refused"), FeelComfort::SaveScales(NotStorage, nullptr, Saved));

	return true;
}

#undef FEEL_TEST_FLAGS

#endif // WITH_DEV_AUTOMATION_TESTS
