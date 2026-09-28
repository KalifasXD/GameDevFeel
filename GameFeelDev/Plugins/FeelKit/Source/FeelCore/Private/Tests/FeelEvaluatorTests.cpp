// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "FeelEvaluator.h"
#include "FeelFrameOutput.h"
#include "FeelRecipe.h"
#include "Steps/FeelStep_ProceduralShake.h"
#include "Steps/FeelStep_ScalePunch.h"
#include "Steps/FeelStep_ScreenFlash.h"
#include "UObject/Package.h"

#define FEEL_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

namespace FeelEvaluatorTests
{
	UFeelRecipe* MakeRecipe()
	{
		return NewObject<UFeelRecipe>(GetTransientPackage());
	}

	void SetConstantCurve(FFeelTrack& Track)
	{
		Track.IntensityCurve.GetRichCurve()->Reset();
	}

	void SetLinearCurve(FFeelTrack& Track, float ValueAtStart, float ValueAtEnd)
	{
		FRichCurve* Curve = Track.IntensityCurve.GetRichCurve();
		Curve->Reset();
		Curve->SetKeyInterpMode(Curve->AddKey(0.0f, ValueAtStart), RCIM_Linear);
		Curve->SetKeyInterpMode(Curve->AddKey(1.0f, ValueAtEnd), RCIM_Linear);
	}

	/** Adds a white flash at full opacity with a constant curve, so the flash alpha equals the track intensity. */
	int32 AddFlashTrack(UFeelRecipe* Recipe, float StartTime, float Duration)
	{
		FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
		UFeelStep_ScreenFlash* Flash = NewObject<UFeelStep_ScreenFlash>(Recipe);
		Flash->MaxOpacity = 1.0f;
		Track.Step = Flash;
		Track.StartTime = StartTime;
		Track.Duration = Duration;
		SetConstantCurve(Track);
		return Recipe->Tracks.Num() - 1;
	}

	int32 AddShakeTrack(UFeelRecipe* Recipe, EFeelShakeMode Mode, float StartTime, float Duration)
	{
		FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
		UFeelStep_ProceduralShake* Shake = NewObject<UFeelStep_ProceduralShake>(Recipe);
		Shake->Mode = Mode;
		Shake->FieldOfViewAmplitude = 2.0f;
		Track.Step = Shake;
		Track.StartTime = StartTime;
		Track.Duration = Duration;
		SetConstantCurve(Track);
		return Recipe->Tracks.Num() - 1;
	}

	FFeelFrameOutput EvaluateAt(const UFeelRecipe* Recipe, float Time, const FFeelEvalParams& Params = FFeelEvalParams())
	{
		FFeelOutputAccumulator Accumulator;
		FFeelEvaluator::Evaluate(*Recipe, Time, Params, Accumulator);
		return Accumulator.Output;
	}

	bool OutputsEqual(const FFeelFrameOutput& A, const FFeelFrameOutput& B)
	{
		return A.CameraLocationOffset == B.CameraLocationOffset
			&& A.CameraRotationOffset == B.CameraRotationOffset
			&& A.FieldOfViewOffset == B.FieldOfViewOffset
			&& A.FlashAlpha == B.FlashAlpha
			&& A.TargetScaleDelta == B.TargetScaleDelta;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelEvaluatorTrackBoundariesTest, "FeelKit.Evaluator.TrackBoundaries", FEEL_TEST_FLAGS)
bool FFeelEvaluatorTrackBoundariesTest::RunTest(const FString& Parameters)
{
	using namespace FeelEvaluatorTests;

	UFeelRecipe* Recipe = MakeRecipe();
	AddFlashTrack(Recipe, 1.0f, 2.0f);
	const FFeelTrack& Track = Recipe->Tracks[0];

	TestFalse(TEXT("Inactive just before start"), FFeelEvaluator::IsTrackActiveAt(Track, 0.999f));
	TestTrue(TEXT("Active at start"), FFeelEvaluator::IsTrackActiveAt(Track, 1.0f));
	TestTrue(TEXT("Active at end"), FFeelEvaluator::IsTrackActiveAt(Track, 3.0f));
	TestFalse(TEXT("Inactive just after end"), FFeelEvaluator::IsTrackActiveAt(Track, 3.001f));

	TestEqual(TEXT("No output before start"), EvaluateAt(Recipe, 0.999f).FlashAlpha, 0.0f);
	TestEqual(TEXT("Output at start"), EvaluateAt(Recipe, 1.0f).FlashAlpha, 1.0f);
	TestEqual(TEXT("Output in the middle"), EvaluateAt(Recipe, 2.0f).FlashAlpha, 1.0f);
	TestEqual(TEXT("Output at end"), EvaluateAt(Recipe, 3.0f).FlashAlpha, 1.0f);
	TestEqual(TEXT("No output after end"), EvaluateAt(Recipe, 3.001f).FlashAlpha, 0.0f);

	TestEqual(TEXT("Alpha at start"), FFeelEvaluator::GetTrackAlpha(Track, 1.0f), 0.0f);
	TestEqual(TEXT("Alpha in the middle"), FFeelEvaluator::GetTrackAlpha(Track, 2.0f), 0.5f);
	TestEqual(TEXT("Alpha at end"), FFeelEvaluator::GetTrackAlpha(Track, 3.0f), 1.0f);

	TestEqual(TEXT("Recipe duration is the latest track end"), Recipe->GetDuration(), 3.0f);

	Recipe->Tracks[0].Duration = 0.0f;
	TestEqual(TEXT("Instant tracks produce no continuous output"), EvaluateAt(Recipe, 1.0f).FlashAlpha, 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelEvaluatorCurveTest, "FeelKit.Evaluator.IntensityCurve", FEEL_TEST_FLAGS)
bool FFeelEvaluatorCurveTest::RunTest(const FString& Parameters)
{
	using namespace FeelEvaluatorTests;

	UFeelRecipe* Recipe = MakeRecipe();
	const int32 Index = AddFlashTrack(Recipe, 0.0f, 2.0f);
	FFeelTrack& Track = Recipe->Tracks[Index];

	SetConstantCurve(Track);
	TestEqual(TEXT("Empty curve evaluates to 1"), Track.EvaluateIntensityCurve(0.5f), 1.0f);

	SetLinearCurve(Track, 0.2f, 1.0f);
	TestEqual(TEXT("Curve at 0"), Track.EvaluateIntensityCurve(0.0f), 0.2f);
	TestEqual(TEXT("Curve at 0.5"), Track.EvaluateIntensityCurve(0.5f), 0.6f);
	TestEqual(TEXT("Curve at 1"), Track.EvaluateIntensityCurve(1.0f), 1.0f);

	SetLinearCurve(Track, 1.0f, 0.0f);
	const FFeelEvalParams Params;
	TestEqual(TEXT("Track intensity at alpha 0"), FFeelEvaluator::ComputeTrackIntensity(*Recipe, Index, 0.0f, Params), 1.0f);
	TestEqual(TEXT("Track intensity at alpha 0.5"), FFeelEvaluator::ComputeTrackIntensity(*Recipe, Index, 1.0f, Params), 0.5f);
	TestEqual(TEXT("Track intensity at alpha 1"), FFeelEvaluator::ComputeTrackIntensity(*Recipe, Index, 2.0f, Params), 0.0f);

	TestEqual(TEXT("Flash follows the curve at alpha 0"), EvaluateAt(Recipe, 0.0f).FlashAlpha, 1.0f);
	TestEqual(TEXT("Flash follows the curve at alpha 0.5"), EvaluateAt(Recipe, 1.0f).FlashAlpha, 0.5f);
	TestEqual(TEXT("Flash follows the curve at alpha 1"), EvaluateAt(Recipe, 2.0f).FlashAlpha, 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelEvaluatorIntensityTest, "FeelKit.Evaluator.IntensityMultiplication", FEEL_TEST_FLAGS)
bool FFeelEvaluatorIntensityTest::RunTest(const FString& Parameters)
{
	using namespace FeelEvaluatorTests;

	UFeelRecipe* Recipe = MakeRecipe();
	Recipe->DefaultIntensity = 0.8f;
	AddFlashTrack(Recipe, 0.0f, 2.0f);

	FFeelEvalParams Params;
	Params.Intensity = 0.5f;

	TestEqual(TEXT("Call x recipe default"), EvaluateAt(Recipe, 1.0f, Params).FlashAlpha, 0.4f);

	SetLinearCurve(Recipe->Tracks[0], 1.0f, 0.0f);
	TestEqual(TEXT("Call x recipe default x curve"), EvaluateAt(Recipe, 1.0f, Params).FlashAlpha, 0.2f);
	TestEqual(TEXT("ComputeTrackIntensity matches"), FFeelEvaluator::ComputeTrackIntensity(*Recipe, 0, 1.0f, Params), 0.2f);

	Params.Intensity = 0.0f;
	FFeelOutputAccumulator Accumulator;
	TestEqual(TEXT("Zero intensity evaluates no tracks"), FFeelEvaluator::Evaluate(*Recipe, 1.0f, Params, Accumulator), 0);
	TestEqual(TEXT("Zero intensity produces no flash"), Accumulator.Output.FlashAlpha, 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelEvaluatorMutedTest, "FeelKit.Evaluator.MutedTracks", FEEL_TEST_FLAGS)
bool FFeelEvaluatorMutedTest::RunTest(const FString& Parameters)
{
	using namespace FeelEvaluatorTests;

	UFeelRecipe* Recipe = MakeRecipe();
	const int32 FlashIndex = AddFlashTrack(Recipe, 0.0f, 1.0f);
	const int32 ShakeIndex = AddShakeTrack(Recipe, EFeelShakeMode::Sine, 0.0f, 1.0f);

	FFeelOutputAccumulator Unmuted;
	TestEqual(TEXT("Both tracks evaluate when enabled"), FFeelEvaluator::Evaluate(*Recipe, 0.1f, FFeelEvalParams(), Unmuted), 2);
	TestTrue(TEXT("Shake produces camera output when enabled"), !Unmuted.Output.CameraLocationOffset.IsNearlyZero());

	Recipe->Tracks[FlashIndex].bEnabled = false;
	Recipe->Tracks[ShakeIndex].bEnabled = false;

	FFeelOutputAccumulator Muted;
	TestEqual(TEXT("Muted tracks are not evaluated"), FFeelEvaluator::Evaluate(*Recipe, 0.1f, FFeelEvalParams(), Muted), 0);
	TestEqual(TEXT("Muted flash produces no output"), Muted.Output.FlashAlpha, 0.0f);
	TestTrue(TEXT("Muted shake produces no camera output"), Muted.Output.CameraLocationOffset.IsZero() && Muted.Output.CameraRotationOffset.IsZero());
	TestEqual(TEXT("Muted shake produces no FOV output"), Muted.Output.FieldOfViewOffset, 0.0f);

	Recipe->Tracks[FlashIndex].bEnabled = true;
	Recipe->Tracks[FlashIndex].Step = nullptr;
	TestEqual(TEXT("Tracks without a step produce no output"), EvaluateAt(Recipe, 0.1f).FlashAlpha, 0.0f);

	return true;
}

#if WITH_EDITORONLY_DATA
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelEvaluatorSoloTest, "FeelKit.Evaluator.Solo", FEEL_TEST_FLAGS)
bool FFeelEvaluatorSoloTest::RunTest(const FString& Parameters)
{
	using namespace FeelEvaluatorTests;

	UFeelRecipe* Recipe = MakeRecipe();
	AddFlashTrack(Recipe, 0.0f, 1.0f);
	const int32 ShakeIndex = AddShakeTrack(Recipe, EFeelShakeMode::Sine, 0.0f, 1.0f);
	Recipe->Tracks[ShakeIndex].bSolo = true;

	FFeelEvalParams Params;
	Params.bRespectSolo = true;
	const FFeelFrameOutput Soloed = EvaluateAt(Recipe, 0.1f, Params);
	TestEqual(TEXT("Non-soloed flash is silent while soloing"), Soloed.FlashAlpha, 0.0f);
	TestTrue(TEXT("Soloed shake still plays"), !Soloed.CameraLocationOffset.IsNearlyZero());

	TestEqual(TEXT("Solo is ignored unless requested"), EvaluateAt(Recipe, 0.1f).FlashAlpha, 1.0f);

	return true;
}
#endif

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelEvaluatorDeterminismTest, "FeelKit.Evaluator.Determinism", FEEL_TEST_FLAGS)
bool FFeelEvaluatorDeterminismTest::RunTest(const FString& Parameters)
{
	using namespace FeelEvaluatorTests;

	UFeelRecipe* Recipe = MakeRecipe();
	AddShakeTrack(Recipe, EFeelShakeMode::Perlin, 0.0f, 2.0f);
	AddShakeTrack(Recipe, EFeelShakeMode::Sine, 0.0f, 2.0f);
	AddShakeTrack(Recipe, EFeelShakeMode::Directional, 0.0f, 2.0f);
	AddFlashTrack(Recipe, 0.0f, 2.0f);
	{
		FFeelTrack& PunchTrack = Recipe->Tracks.AddDefaulted_GetRef();
		PunchTrack.Step = NewObject<UFeelStep_ScalePunch>(Recipe);
		PunchTrack.Duration = 2.0f;
	}
	Recipe->Tracks[0].Seed = 7;

	const TArray<float> Times = { 0.05f, 0.37f, 0.81f, 1.12f, 1.6f, 1.99f };

	TArray<FFeelFrameOutput> Forward;
	for (float Time : Times)
	{
		Forward.Add(EvaluateAt(Recipe, Time));
	}

	for (int32 Index = Times.Num() - 1; Index >= 0; --Index)
	{
		const FFeelFrameOutput Backward = EvaluateAt(Recipe, Times[Index]);
		TestTrue(FString::Printf(TEXT("Same output scrubbing backwards at t=%.2f"), Times[Index]), OutputsEqual(Forward[Index], Backward));
	}

	TestTrue(TEXT("Output changes over time"), !OutputsEqual(Forward[0], Forward[1]));

	FFeelEvalParams OtherInstance;
	OtherInstance.InstanceSeed = 12345;
	TestTrue(TEXT("A different instance seed changes the shake"), !OutputsEqual(Forward[2], EvaluateAt(Recipe, Times[2], OtherInstance)));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelOutputAccumulatorTest, "FeelKit.Output.Accumulator", FEEL_TEST_FLAGS)
bool FFeelOutputAccumulatorTest::RunTest(const FString& Parameters)
{
	FFeelOutputAccumulator Accumulator;
	Accumulator.AddCameraOffset(FVector(1.0, 2.0, 3.0), FRotator(1.0, 0.0, 0.0));
	Accumulator.AddCameraOffset(FVector(1.0, 0.0, 0.0), FRotator(0.0, 2.0, 0.0));
	Accumulator.AddScreenFlash(FLinearColor::Red, 0.3f);
	Accumulator.AddScreenFlash(FLinearColor::Blue, 0.7f);
	Accumulator.AddScreenFlash(FLinearColor::Green, 0.5f);
	Accumulator.AddTargetScale(FVector(0.5));

	const FFeelFrameOutput& Output = Accumulator.Output;
	TestEqual(TEXT("Camera offsets add"), Output.CameraLocationOffset, FVector(2.0, 2.0, 3.0));
	TestEqual(TEXT("Rotation offsets add"), Output.CameraRotationOffset, FRotator(1.0, 2.0, 0.0));
	TestEqual(TEXT("Strongest flash wins"), Output.FlashAlpha, 0.7f);
	TestEqual(TEXT("Strongest flash keeps its color"), Output.FlashColor, FLinearColor::Blue);
	TestEqual(TEXT("Scale delta applies relative to base"), Output.ApplyToScale(FVector(2.0)), FVector(3.0));

	FVector Location = FVector::ZeroVector;
	FRotator Rotation = FRotator(0.0, 90.0, 0.0);
	float FieldOfView = 90.0f;
	FFeelFrameOutput Forward;
	Forward.CameraLocationOffset = FVector(10.0, 0.0, 0.0);
	Forward.FieldOfViewOffset = 5.0f;
	Forward.ApplyToView(Location, Rotation, FieldOfView);
	TestTrue(TEXT("Location offset is camera-relative"), Location.Equals(FVector(0.0, 10.0, 0.0), 0.001));
	TestEqual(TEXT("FOV offset applies"), FieldOfView, 95.0f);

	return true;
}

#undef FEEL_TEST_FLAGS

#endif // WITH_DEV_AUTOMATION_TESTS
