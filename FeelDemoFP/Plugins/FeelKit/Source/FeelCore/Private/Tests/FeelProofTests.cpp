// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "FeelComfortTypes.h"
#include "FeelEvaluator.h"
#include "FeelFrameOutput.h"
#include "FeelPlayCapture.h"
#include "FeelRecipe.h"
#include "FeelSettings.h"
#include "FeelSubsystem.h"
#include "FeelTags.h"
#include "Misc/App.h"
#include "Steps/FeelStep_ScreenFlash.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"

#define FEEL_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

namespace FeelProofTests
{
	class FScopedWorld
	{
	public:
		FScopedWorld()
			: PreviousDeltaTime(FApp::GetDeltaTime())
		{
			World = UWorld::CreateWorld(EWorldType::Game, false);
			FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
			World->BeginPlay();
		}

		~FScopedWorld()
		{
			FApp::SetDeltaTime(PreviousDeltaTime);
			World->RemoveFromRoot();
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}

		UFeelSubsystem* Subsystem() const { return World->GetSubsystem<UFeelSubsystem>(); }

		void Step(float Seconds, int32 Count = 1)
		{
			for (int32 Index = 0; Index < Count; ++Index)
			{
				FApp::SetDeltaTime(Seconds);
				Subsystem()->Tick(Seconds);
			}
		}

		UWorld* World = nullptr;

	private:
		double PreviousDeltaTime = 0.0;
	};

	UFeelRecipe* MakeFlashRecipe(float Duration)
	{
		UFeelRecipe* Recipe = NewObject<UFeelRecipe>(GetTransientPackage());
		FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
		UFeelStep_ScreenFlash* Flash = NewObject<UFeelStep_ScreenFlash>(Recipe);
		Flash->MaxOpacity = 1.0f;
		Track.Step = Flash;
		Track.Channel = FeelTags::Screen_Flash;
		Track.Duration = Duration;
		Track.IntensityCurve.GetRichCurve()->Reset();
		return Recipe;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelFlashLimiterTest, "FeelKit.Comfort.FlashLimiter", FEEL_TEST_FLAGS)
bool FFeelFlashLimiterTest::RunTest(const FString& Parameters)
{
	FFeelComfortScales Scales;
	Scales.bLimitFlashes = true;
	Scales.MaxFlashesPerSecond = 3.0f;
	Scales.FlashLimitMode = EFeelFlashLimitMode::Soften;
	Scales.SoftenedFlashScale = 0.3f;

	FFeelFlashLimiter Limiter;
	TestEqual(TEXT("First flash is full strength"), Limiter.RegisterFlash(0.0, Scales), 1.0f);
	TestEqual(TEXT("Second flash is full strength"), Limiter.RegisterFlash(0.1, Scales), 1.0f);
	TestEqual(TEXT("Third flash is full strength"), Limiter.RegisterFlash(0.2, Scales), 1.0f);
	TestEqual(TEXT("A fourth flash within the second is softened"), Limiter.RegisterFlash(0.3, Scales), 0.3f);
	TestEqual(TEXT("Once the window has passed, flashes are full again"), Limiter.RegisterFlash(1.5, Scales), 1.0f);

	Scales.FlashLimitMode = EFeelFlashLimitMode::Suppress;
	Limiter.Reset();
	Limiter.RegisterFlash(0.0, Scales);
	Limiter.RegisterFlash(0.1, Scales);
	Limiter.RegisterFlash(0.2, Scales);
	TestEqual(TEXT("Suppress removes the extra flash"), Limiter.RegisterFlash(0.3, Scales), 0.0f);
	TestEqual(TEXT("Suppressed flashes do not count, so the next free slot is at 1 s"), Limiter.RegisterFlash(1.0, Scales), 1.0f);

	Scales.MaxFlashesPerSecond = 0.5f;
	Limiter.Reset();
	TestEqual(TEXT("Slow rate: first flash plays"), Limiter.RegisterFlash(0.0, Scales), 1.0f);
	TestEqual(TEXT("Slow rate: a flash 1.5 s later is suppressed"), Limiter.RegisterFlash(1.5, Scales), 0.0f);
	TestEqual(TEXT("Slow rate: a flash 2 s later plays"), Limiter.RegisterFlash(2.0, Scales), 1.0f);

	Scales.bLimitFlashes = false;
	Limiter.Reset();
	for (int32 Index = 0; Index < 10; ++Index)
	{
		TestEqual(TEXT("Limiter off: every flash plays"), Limiter.RegisterFlash(Index * 0.01, Scales), 1.0f);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelMotionComfortTest, "FeelKit.Comfort.MotionComfort", FEEL_TEST_FLAGS)
bool FFeelMotionComfortTest::RunTest(const FString& Parameters)
{
	FFeelComfortScales Scales;
	FRotator Rotation(5.0, 10.0, 20.0);
	float FieldOfView = 30.0f;
	float Previous = 0.0f;
	FeelComfort::ApplyMotionComfort(Scales, Rotation, FieldOfView, Previous, 0.1f);
	TestEqual(TEXT("Defaults keep roll"), Rotation.Roll, 20.0);
	TestEqual(TEXT("Defaults do not limit FOV changes"), FieldOfView, 30.0f);

	Scales.bAllowCameraRoll = false;
	Scales.MaxFieldOfViewChangePerSecond = 40.0f;
	Rotation = FRotator(5.0, 10.0, 20.0);
	FieldOfView = 30.0f;
	Previous = 0.0f;
	FeelComfort::ApplyMotionComfort(Scales, Rotation, FieldOfView, Previous, 0.1f);
	TestEqual(TEXT("Roll removed"), Rotation.Roll, 0.0);
	TestEqual(TEXT("Pitch kept"), Rotation.Pitch, 5.0);
	TestEqual(TEXT("FOV change limited to 40 deg/s x 0.1 s"), FieldOfView, 4.0f, 0.001f);
	TestEqual(TEXT("Previous offset remembered"), Previous, 4.0f, 0.001f);

	FieldOfView = 30.0f;
	FeelComfort::ApplyMotionComfort(Scales, Rotation, FieldOfView, Previous, 0.1f);
	TestEqual(TEXT("Next frame moves another 4 degrees"), FieldOfView, 8.0f, 0.001f);

	FieldOfView = 0.0f;
	FeelComfort::ApplyMotionComfort(Scales, Rotation, FieldOfView, Previous, 0.0f);
	TestEqual(TEXT("A jump without elapsed time (scrubbing) is not limited"), FieldOfView, 0.0f, 0.001f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelTrackScalesTest, "FeelKit.Evaluator.TrackScales", FEEL_TEST_FLAGS)
bool FFeelTrackScalesTest::RunTest(const FString& Parameters)
{
	TStrongObjectPtr<UFeelRecipe> Recipe(FeelProofTests::MakeFlashRecipe(1.0f));

	FFeelEvalParams Params;
	FFeelOutputAccumulator Full;
	FFeelEvaluator::Evaluate(*Recipe, 0.5f, Params, Full);
	TestEqual(TEXT("No track scales: full flash"), Full.Output.FlashAlpha, 1.0f, 0.001f);

	const TArray<float> Softened = { 0.3f };
	Params.TrackScales = Softened;
	FFeelOutputAccumulator Soft;
	FFeelEvaluator::Evaluate(*Recipe, 0.5f, Params, Soft);
	TestEqual(TEXT("Track scale softens the flash"), Soft.Output.FlashAlpha, 0.3f, 0.001f);

	const TArray<float> Suppressed = { 0.0f };
	Params.TrackScales = Suppressed;
	FFeelOutputAccumulator None;
	FFeelEvaluator::Evaluate(*Recipe, 0.5f, Params, None);
	TestEqual(TEXT("Track scale 0 removes the flash"), None.Output.FlashAlpha, 0.0f, 0.001f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelPlayCaptureStoreTest, "FeelKit.Proof.CaptureStore", FEEL_TEST_FLAGS)
bool FFeelPlayCaptureStoreTest::RunTest(const FString& Parameters)
{
	FFeelPlayCaptureStore& Store = FFeelPlayCaptureStore::Get();
	const TArray<FFeelPlayCapture> Saved = Store.GetCaptures();
	Store.Clear();

	int32 ChangeCount = 0;
	const FDelegateHandle Handle = Store.OnChanged.AddLambda([&ChangeCount]() { ++ChangeCount; });
	for (int32 Index = 0; Index < FFeelPlayCaptureStore::MaxCaptures + 6; ++Index)
	{
		FFeelPlayCapture Capture;
		Capture.Seed = Index;
		Store.Add(MoveTemp(Capture));
	}
	TestEqual(TEXT("The store keeps at most MaxCaptures plays"), Store.GetCaptures().Num(), FFeelPlayCaptureStore::MaxCaptures);
	TestEqual(TEXT("Oldest plays are dropped"), Store.GetCaptures()[0].Seed, 6);
	TestEqual(TEXT("Newest play is last"), Store.GetCaptures().Last().Seed, FFeelPlayCaptureStore::MaxCaptures + 5);
	TestEqual(TEXT("Every add notifies listeners"), ChangeCount, FFeelPlayCaptureStore::MaxCaptures + 6);

	Store.Clear();
	TestEqual(TEXT("Clear empties the store"), Store.GetCaptures().Num(), 0);
	Store.OnChanged.Remove(Handle);

	for (const FFeelPlayCapture& Capture : Saved)
	{
		FFeelPlayCapture Copy = Capture;
		Store.Add(MoveTemp(Copy));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelRuntimeFlashLimiterCaptureTest, "FeelKit.Runtime.FlashLimiterAndCapture", FEEL_TEST_FLAGS)
bool FFeelRuntimeFlashLimiterCaptureTest::RunTest(const FString& Parameters)
{
	using namespace FeelProofTests;

	const FFeelComfortScales& Defaults = GetDefault<UFeelSettings>()->DefaultComfortScales;
	if (!Defaults.bLimitFlashes || Defaults.MaxFlashesPerSecond < 1.0f || Defaults.MaxFlashesPerSecond >= 4.0f)
	{
		AddInfo(TEXT("Project default comfort does not limit flashes to 1-3 per second; runtime limiter checks skipped."));
		return true;
	}
	const int32 Allowed = FMath::FloorToInt32(Defaults.MaxFlashesPerSecond + 0.001f);

	FFeelPlayCaptureStore::Get().Clear();
	FScopedWorld TestWorld;
	UFeelSubsystem* Subsystem = TestWorld.Subsystem();
	TStrongObjectPtr<UFeelRecipe> Recipe(MakeFlashRecipe(0.5f));

	for (int32 Play = 0; Play <= Allowed; ++Play)
	{
		Subsystem->PlayFeel(Recipe.Get(), FFeelTarget(), 0.8f);
		TestWorld.Step(0.02f);
	}

	const TArray<FFeelInstance>& Instances = Subsystem->GetInstances();
	if (!TestEqual(TEXT("Every play is running"), Instances.Num(), Allowed + 1))
	{
		return false;
	}
	for (int32 Play = 0; Play < Allowed; ++Play)
	{
		TestEqual(TEXT("Plays within the allowed rate keep full strength"), Instances[Play].TrackScales.IsValidIndex(0) ? Instances[Play].TrackScales[0] : -1.0f, 1.0f);
	}
	const float ExpectedExtra = Defaults.FlashLimitMode == EFeelFlashLimitMode::Suppress ? 0.0f : Defaults.SoftenedFlashScale;
	TestEqual(TEXT("The extra flash is limited"), Instances[Allowed].TrackScales.IsValidIndex(0) ? Instances[Allowed].TrackScales[0] : -1.0f, ExpectedExtra, 0.001f);
	const int32 ExtraSeed = Instances[Allowed].Seed;

	TestWorld.Step(0.1f, 10);
	TestEqual(TEXT("Every play finished"), Subsystem->GetNumActiveInstances(), 0);

	const TArray<FFeelPlayCapture>& Captures = FFeelPlayCaptureStore::Get().GetCaptures();
#if UE_BUILD_SHIPPING
	TestEqual(TEXT("Shipping records nothing"), Captures.Num(), 0);
#else
	if (TestEqual(TEXT("Each finished play is recorded"), Captures.Num(), Allowed + 1))
	{
		const FFeelPlayCapture& Extra = Captures.Last();
		TestTrue(TEXT("The capture points at the recipe"), Extra.Recipe.Get() == Recipe.Get());
		TestEqual(TEXT("The capture keeps the seed"), Extra.Seed, ExtraSeed);
		TestEqual(TEXT("The capture keeps the intensity"), Extra.Intensity, 0.8f);
		TestTrue(TEXT("The capture keeps the comfort"), Extra.bHasComfort);
		TestEqual(TEXT("The capture keeps the limiter decision"), Extra.TrackScales.IsValidIndex(0) ? Extra.TrackScales[0] : -1.0f, ExpectedExtra, 0.001f);
		TestFalse(TEXT("A play that ran to its end is not interrupted"), Extra.bInterrupted);

		// Replaying: the same inputs give the same frame.
		FFeelEvalParams Params;
		Params.InstanceSeed = Extra.Seed;
		Params.Intensity = Extra.Intensity;
		Params.TrackScales = Extra.TrackScales;
		Params.Comfort.Scales = &Extra.ComfortScales;
		Params.Comfort.Mappings = GetDefault<UFeelSettings>()->ChannelComfortGroups;
		FFeelOutputAccumulator Replay;
		FFeelEvaluator::Evaluate(*Recipe, 0.25f, Params, Replay);
		const float ComfortScale = Extra.ComfortScales.Master * Extra.ComfortScales.Flashes;
		TestEqual(TEXT("Replayed frame applies intensity, comfort and limiter"), Replay.Output.FlashAlpha, FMath::Min(0.8f * ExpectedExtra * ComfortScale, 1.0f), 0.001f);
	}
#endif
	FFeelPlayCaptureStore::Get().Clear();
	return true;
}

#endif
