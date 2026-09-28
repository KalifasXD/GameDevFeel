// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "FeelComfortTypes.h"
#include "FeelEvaluator.h"
#include "FeelRecipe.h"
#include "FeelTags.h"
#include "FeelTrackLifecycle.h"
#include "Steps/FeelStep_BlueprintEvent.h"
#include "Steps/FeelStep_PlaySound.h"
#include "Tests/FeelLifecycleTestTypes.h"
#include "UObject/Package.h"

#define FEEL_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

namespace FeelLifecycleTests
{
	UFeelTestRecorderStep* AddRecorderTrack(UFeelRecipe* Recipe, float StartTime, float Duration)
	{
		UFeelTestRecorderStep* Step = NewObject<UFeelTestRecorderStep>(Recipe);
		FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
		Track.Step = Step;
		Track.StartTime = StartTime;
		Track.Duration = Duration;
		Track.IntensityCurve.GetRichCurve()->Reset();
		return Step;
	}

	FFeelContext MakeContext(int32 TrackIndex, float Intensity)
	{
		FFeelContext Context;
		Context.TrackIndex = TrackIndex;
		Context.Intensity = Intensity;
		return Context;
	}

	void Update(FFeelTrackLifecycle& Lifecycle, const UFeelRecipe* Recipe, float Time, const FFeelEvalParams& Params = FFeelEvalParams())
	{
		Lifecycle.Update(*Recipe, Time, Params, &MakeContext);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelLifecycleStartStopTest, "FeelKit.Lifecycle.StartStop", FEEL_TEST_FLAGS)
bool FFeelLifecycleStartStopTest::RunTest(const FString& Parameters)
{
	using namespace FeelLifecycleTests;

	UFeelRecipe* Recipe = NewObject<UFeelRecipe>(GetTransientPackage());
	UFeelTestRecorderStep* Long = AddRecorderTrack(Recipe, 0.2f, 0.5f);
	UFeelTestRecorderStep* Instant = AddRecorderTrack(Recipe, 0.3f, 0.0f);
	UFeelTestRecorderStep* Short = AddRecorderTrack(Recipe, 0.35f, 0.1f);

	FFeelTrackLifecycle Lifecycle;
	Lifecycle.Reset(Recipe->Tracks.Num());

	Update(Lifecycle, Recipe, 0.1f);
	TestEqual(TEXT("Nothing starts before its start time"), Long->StartCount, 0);

	Update(Lifecycle, Recipe, 0.25f);
	TestEqual(TEXT("Long track starts"), Long->StartCount, 1);
	TestEqual(TEXT("Long track has not stopped"), Long->StopCount, 0);
	TestEqual(TEXT("Context carries the track index"), Long->LastTrackIndex, 0);
	TestEqual(TEXT("Context carries the start intensity"), Long->LastStartIntensity, 1.0f);
	TestEqual(TEXT("One track running"), Lifecycle.GetNumRunning(), 1);

	Update(Lifecycle, Recipe, 0.32f);
	TestEqual(TEXT("Instant track fires once"), Instant->StartCount, 1);
	TestEqual(TEXT("Instant track stops right away"), Instant->StopCount, 1);

	Update(Lifecycle, Recipe, 0.6f);
	TestEqual(TEXT("A track crossed within one update starts"), Short->StartCount, 1);
	TestEqual(TEXT("A track crossed within one update stops"), Short->StopCount, 1);
	TestEqual(TEXT("Long track still running"), Long->StopCount, 0);

	Update(Lifecycle, Recipe, 0.8f);
	Update(Lifecycle, Recipe, 1.0f);
	TestEqual(TEXT("Long track stops once after its end"), Long->StopCount, 1);
	TestEqual(TEXT("A normal end is not an interruption"), Long->InterruptedStopCount, 0);
	TestEqual(TEXT("Instant track never fires twice"), Instant->StartCount, 1);
	TestEqual(TEXT("Nothing running at the end"), Lifecycle.GetNumRunning(), 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelLifecycleResumeTest, "FeelKit.Lifecycle.ResumeSkipsPassedTracks", FEEL_TEST_FLAGS)
bool FFeelLifecycleResumeTest::RunTest(const FString& Parameters)
{
	using namespace FeelLifecycleTests;

	UFeelRecipe* Recipe = NewObject<UFeelRecipe>(GetTransientPackage());
	UFeelTestRecorderStep* Long = AddRecorderTrack(Recipe, 0.2f, 0.5f);
	UFeelTestRecorderStep* Instant = AddRecorderTrack(Recipe, 0.3f, 0.0f);
	UFeelTestRecorderStep* Short = AddRecorderTrack(Recipe, 0.35f, 0.1f);

	FFeelTrackLifecycle Lifecycle;
	Lifecycle.Reset(Recipe->Tracks.Num(), 0.5f);
	Update(Lifecycle, Recipe, 0.55f);

	TestEqual(TEXT("A track still active at the resume point starts"), Long->StartCount, 1);
	TestEqual(TEXT("An instant track before the resume point does not fire"), Instant->StartCount, 0);
	TestEqual(TEXT("A track already over at the resume point is skipped"), Short->StartCount, 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelLifecycleFiltersTest, "FeelKit.Lifecycle.MuteComfortSubstituteStop", FEEL_TEST_FLAGS)
bool FFeelLifecycleFiltersTest::RunTest(const FString& Parameters)
{
	using namespace FeelLifecycleTests;

	UFeelRecipe* Recipe = NewObject<UFeelRecipe>(GetTransientPackage());
	UFeelTestRecorderStep* Muted = AddRecorderTrack(Recipe, 0.0f, 1.0f);
	Recipe->Tracks[0].bEnabled = false;

	UFeelTestRecorderStep* ComfortOff = AddRecorderTrack(Recipe, 0.0f, 1.0f);
	Recipe->Tracks[1].Channel = FeelTags::Screen_Flash;

	UFeelTestRecorderStep* Original = AddRecorderTrack(Recipe, 0.0f, 1.0f);
	UFeelTestRecorderStep* Substitute = NewObject<UFeelTestRecorderStep>(Recipe);
	Recipe->Tracks[2].Channel = FeelTags::Screen_Flash;
	Recipe->Tracks[2].bEssential = true;
	Recipe->Tracks[2].SubstituteStep = Substitute;

	UFeelTestRecorderStep* Plain = AddRecorderTrack(Recipe, 0.0f, 1.0f);

	const TArray<FFeelChannelComfortMapping> Mappings = { FFeelChannelComfortMapping(FeelTags::Screen_Flash, EFeelComfortGroup::Flashes) };
	FFeelComfortScales Scales;
	Scales.Flashes = 0.0f;

	FFeelEvalParams Params;
	Params.Intensity = 0.5f;
	Params.Comfort.Scales = &Scales;
	Params.Comfort.Mappings = Mappings;

	FFeelTrackLifecycle Lifecycle;
	Lifecycle.Reset(Recipe->Tracks.Num());
	Update(Lifecycle, Recipe, 0.1f, Params);

	TestEqual(TEXT("Muted tracks do not start"), Muted->StartCount, 0);
	TestEqual(TEXT("Tracks on a disabled comfort channel do not start"), ComfortOff->StartCount, 0);
	TestEqual(TEXT("An essential track's original does not start while its channel is off"), Original->StartCount, 0);
	TestEqual(TEXT("Its substitute starts instead"), Substitute->StartCount, 1);
	TestEqual(TEXT("Call intensity reaches the context"), Plain->LastStartIntensity, 0.5f);

	Lifecycle.StopAll(Recipe, true, &MakeContext);
	TestEqual(TEXT("StopAll stops the substitute"), Substitute->InterruptedStopCount, 1);
	TestEqual(TEXT("StopAll stops the plain track"), Plain->InterruptedStopCount, 1);
	TestEqual(TEXT("Tracks that never started are not stopped"), Muted->StopCount, 0);
	TestEqual(TEXT("Nothing running after StopAll"), Lifecycle.GetNumRunning(), 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelBlueprintEventTest, "FeelKit.Steps.BlueprintEvent", FEEL_TEST_FLAGS)
bool FFeelBlueprintEventTest::RunTest(const FString& Parameters)
{
	UFeelTestEventReceiver* Receiver = NewObject<UFeelTestEventReceiver>(GetTransientPackage());

	TestTrue(TEXT("Calls an event without inputs"), UFeelStep_BlueprintEvent::CallEvent(Receiver, TEXT("FeelTestNoInputs"), 0.7f));
	TestEqual(TEXT("Event without inputs ran"), Receiver->NoInputCalls, 1);

	TestTrue(TEXT("Calls an event with a float input"), UFeelStep_BlueprintEvent::CallEvent(Receiver, TEXT("FeelTestFloat"), 0.7f));
	TestEqual(TEXT("Float input receives the intensity"), Receiver->LastFloat, 0.7f);

	TestTrue(TEXT("Calls an event with a Blueprint double input"), UFeelStep_BlueprintEvent::CallEvent(Receiver, TEXT("FeelTestDouble"), 0.25f));
	TestEqual(TEXT("Double input receives the intensity"), Receiver->LastDouble, 0.25, 0.0001);

	TestFalse(TEXT("No receiver is refused quietly"), UFeelStep_BlueprintEvent::CallEvent(nullptr, TEXT("FeelTestNoInputs"), 1.0f));

	UFeelStep_BlueprintEvent* Step = NewObject<UFeelStep_BlueprintEvent>(GetTransientPackage());
	Step->StartEventName = TEXT("FeelTestNoInputs");
	Step->OnStart(FFeelContext());
	TestFalse(TEXT("Blueprint events cannot preview"), Step->SupportsPreview());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelPlaySoundTest, "FeelKit.Steps.PlaySound", FEEL_TEST_FLAGS)
bool FFeelPlaySoundTest::RunTest(const FString& Parameters)
{
	UFeelStep_PlaySound* Step = NewObject<UFeelStep_PlaySound>(GetTransientPackage());
	TestTrue(TEXT("Sounds play in the preview"), Step->SupportsPreview());
	TestTrue(TEXT("Default placement is 2D"), Step->Placement == EFeelSoundPlacement::TwoD);
	TestTrue(TEXT("Sounds default to the Audio channel"), Step->GetDefaultChannel() == FeelTags::Audio.GetTag());

	FFeelContext Context;
	Context.InstanceId = 3;
	Context.TrackIndex = 1;
	Context.TrackDuration = 0.5f;
	Step->OnStart(Context);
	Step->OnStop(Context, true);
	Step->OnStop(Context, false);
	TestTrue(TEXT("Starting and stopping without a sound or world is safe"), true);

	return true;
}

#undef FEEL_TEST_FLAGS

#endif // WITH_DEV_AUTOMATION_TESTS
