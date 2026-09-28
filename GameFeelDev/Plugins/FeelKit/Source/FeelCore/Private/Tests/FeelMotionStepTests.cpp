// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "FeelFrameOutput.h"
#include "FeelMotion.h"
#include "Steps/FeelStep_CameraPunch.h"
#include "Steps/FeelStep_FOVKick.h"
#include "Steps/FeelStep_Hitstop.h"
#include "Steps/FeelStep_SlowMoRamp.h"
#include "Steps/FeelStep_SquashStretch.h"
#include "UObject/Package.h"

#define FEEL_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

namespace FeelMotionStepTests
{
	FFeelStepEvalContext MakeContext(float LocalTime, float Duration, float Intensity = 1.0f, int32 Seed = 0)
	{
		FFeelStepEvalContext Context;
		Context.LocalTime = LocalTime;
		Context.Duration = Duration;
		Context.Alpha = Duration > 0.0f ? FMath::Clamp(LocalTime / Duration, 0.0f, 1.0f) : 0.0f;
		Context.Intensity = Intensity;
		Context.Seed = Seed;
		return Context;
	}

	FFeelFrameOutput EvaluateStep(const UFeelStep* Step, const FFeelStepEvalContext& Context)
	{
		FFeelOutputAccumulator Accumulator;
		Step->Evaluate(Context, Accumulator);
		return Accumulator.Output;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelMotionCurvesTest, "FeelKit.Motion.Curves", FEEL_TEST_FLAGS)
bool FFeelMotionCurvesTest::RunTest(const FString& Parameters)
{
	const float PeakTime = FeelMotion::SpringPeakTime(5.0f, 7.0f);
	TestEqual(TEXT("Spring starts at 0"), FeelMotion::Spring(0.0f, 5.0f, 7.0f), 0.0f);
	TestEqual(TEXT("Spring first peak is exactly 1"), FeelMotion::Spring(PeakTime, 5.0f, 7.0f), 1.0f, 0.001f);
	TestTrue(TEXT("Spring rises to the peak"), FeelMotion::Spring(PeakTime * 0.8f, 5.0f, 7.0f) < 1.0f);
	TestTrue(TEXT("Spring falls after the peak"), FeelMotion::Spring(PeakTime * 1.2f, 5.0f, 7.0f) < 1.0f);
	TestTrue(TEXT("Spring overshoots below zero"), FeelMotion::Spring(0.15f, 5.0f, 7.0f) < 0.0f);
	TestTrue(TEXT("Spring settles"), FMath::Abs(FeelMotion::Spring(1.5f, 5.0f, 7.0f)) < 0.01f);

	TestEqual(TEXT("Kick starts at 0"), FeelMotion::Kick(0.0f, 0.15f), 0.0f);
	TestEqual(TEXT("Kick peaks at the attack fraction"), FeelMotion::Kick(0.15f, 0.15f), 1.0f, 0.001f);
	TestEqual(TEXT("Kick ends at 0"), FeelMotion::Kick(1.0f, 0.15f), 0.0f, 0.001f);
	TestTrue(TEXT("Kick is partway up during the attack"), FeelMotion::Kick(0.075f, 0.15f) > 0.5f && FeelMotion::Kick(0.075f, 0.15f) < 1.0f);

	TestEqual(TEXT("Smooth peaks in the middle"), FeelMotion::Smooth(0.5f), 1.0f, 0.001f);
	TestEqual(TEXT("Smooth ends at 0"), FeelMotion::Smooth(1.0f), 0.0f, 0.001f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelCameraPunchTest, "FeelKit.Steps.CameraPunch", FEEL_TEST_FLAGS)
bool FFeelCameraPunchTest::RunTest(const FString& Parameters)
{
	using namespace FeelMotionStepTests;

	UFeelStep_CameraPunch* Punch = NewObject<UFeelStep_CameraPunch>(GetTransientPackage());
	Punch->Shape = EFeelMotionShape::Spring;
	Punch->Frequency = 5.0f;
	Punch->Damping = 7.0f;
	const float PeakTime = FeelMotion::SpringPeakTime(5.0f, 7.0f);

	FFeelFrameOutput Output = EvaluateStep(Punch, MakeContext(PeakTime, 1.0f));
	TestTrue(TEXT("Location reaches the punch at the spring peak"), Output.CameraLocationOffset.Equals(Punch->LocationPunch, 0.01));
	TestTrue(TEXT("Rotation reaches the punch at the spring peak"), Output.CameraRotationOffset.Equals(Punch->RotationPunch, 0.01f));

	Output = EvaluateStep(Punch, MakeContext(PeakTime, 1.0f, 0.5f));
	TestTrue(TEXT("Intensity scales the punch"), Output.CameraLocationOffset.Equals(Punch->LocationPunch * 0.5, 0.01));

	Punch->DirectionJitter = 20.0f;
	const FFeelFrameOutput SeedA = EvaluateStep(Punch, MakeContext(PeakTime, 1.0f, 1.0f, 11));
	const FFeelFrameOutput SeedAAgain = EvaluateStep(Punch, MakeContext(PeakTime, 1.0f, 1.0f, 11));
	const FFeelFrameOutput SeedB = EvaluateStep(Punch, MakeContext(PeakTime, 1.0f, 1.0f, 99));
	TestTrue(TEXT("Jitter keeps the punch strength"), FMath::IsNearlyEqual(SeedA.CameraLocationOffset.Size(), Punch->LocationPunch.Size(), 0.01));
	TestTrue(TEXT("Jitter is repeatable for a seed"), SeedA.CameraLocationOffset.Equals(SeedAAgain.CameraLocationOffset, 0.0001));
	TestFalse(TEXT("Jitter differs between seeds"), SeedA.CameraLocationOffset.Equals(SeedB.CameraLocationOffset, 0.01));

	Punch->DirectionJitter = 0.0f;
	Punch->Shape = EFeelMotionShape::Kick;
	Output = EvaluateStep(Punch, MakeContext(0.15f, 1.0f));
	TestTrue(TEXT("Kick reaches the punch at the attack"), Output.CameraLocationOffset.Equals(Punch->LocationPunch, 0.01));

	Punch->Shape = EFeelMotionShape::Smooth;
	Output = EvaluateStep(Punch, MakeContext(0.5f, 1.0f));
	TestTrue(TEXT("Smooth reaches the punch in the middle"), Output.CameraLocationOffset.Equals(Punch->LocationPunch, 0.01));

	TestTrue(TEXT("Shaped steps start with a flat curve"), Punch->UsesConstantIntensityByDefault());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelFOVKickTest, "FeelKit.Steps.FOVKick", FEEL_TEST_FLAGS)
bool FFeelFOVKickTest::RunTest(const FString& Parameters)
{
	using namespace FeelMotionStepTests;

	UFeelStep_FOVKick* Kick = NewObject<UFeelStep_FOVKick>(GetTransientPackage());
	TestTrue(TEXT("FOV Kick defaults to the kick shape"), Kick->Shape == EFeelMotionShape::Kick);
	TestEqual(TEXT("Full kick at the attack"), EvaluateStep(Kick, MakeContext(0.15f, 1.0f)).FieldOfViewOffset, 8.0f, 0.01f);
	TestEqual(TEXT("Back to normal at the end"), EvaluateStep(Kick, MakeContext(1.0f, 1.0f)).FieldOfViewOffset, 0.0f, 0.01f);

	Kick->FieldOfViewKick = -12.0f;
	TestEqual(TEXT("Negative kick zooms in"), EvaluateStep(Kick, MakeContext(0.15f, 1.0f, 0.5f)).FieldOfViewOffset, -6.0f, 0.01f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelSquashStretchTest, "FeelKit.Steps.SquashStretch", FEEL_TEST_FLAGS)
bool FFeelSquashStretchTest::RunTest(const FString& Parameters)
{
	using namespace FeelMotionStepTests;

	UFeelStep_SquashStretch* Squash = NewObject<UFeelStep_SquashStretch>(GetTransientPackage());
	Squash->Shape = EFeelMotionShape::Smooth;
	Squash->Amount = 0.3f;

	FVector Scale = FVector::OneVector + EvaluateStep(Squash, MakeContext(0.5f, 1.0f)).TargetScaleDelta;
	TestEqual(TEXT("Stretches along Z"), Scale.Z, 1.3, 0.001);
	TestEqual(TEXT("Thins along X"), Scale.X, 1.0 / FMath::Sqrt(1.3), 0.001);
	TestEqual(TEXT("Volume is preserved"), Scale.X * Scale.Y * Scale.Z, 1.0, 0.001);

	Squash->Amount = -0.3f;
	Scale = FVector::OneVector + EvaluateStep(Squash, MakeContext(0.5f, 1.0f)).TargetScaleDelta;
	TestTrue(TEXT("Negative amount squashes and bulges"), Scale.Z < 1.0 && Scale.X > 1.0);
	TestEqual(TEXT("Squash preserves volume"), Scale.X * Scale.Y * Scale.Z, 1.0, 0.001);

	Squash->Amount = 0.3f;
	Squash->bPreserveVolume = false;
	Squash->Axis = EFeelAxis::X;
	Scale = FVector::OneVector + EvaluateStep(Squash, MakeContext(0.5f, 1.0f)).TargetScaleDelta;
	TestEqual(TEXT("Stretches along the chosen axis"), Scale.X, 1.3, 0.001);
	TestEqual(TEXT("Other axes stay without volume preservation"), Scale.Z, 1.0, 0.001);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelSlowMoRampTest, "FeelKit.Steps.SlowMoRamp", FEEL_TEST_FLAGS)
bool FFeelSlowMoRampTest::RunTest(const FString& Parameters)
{
	using namespace FeelMotionStepTests;

	UFeelStep_SlowMoRamp* SlowMo = NewObject<UFeelStep_SlowMoRamp>(GetTransientPackage());
	SlowMo->TimeDilation = 0.3f;
	SlowMo->RampInTime = 0.2f;
	SlowMo->RampOutTime = 0.2f;

	TestEqual(TEXT("Normal speed at the start"), EvaluateStep(SlowMo, MakeContext(0.0f, 1.0f)).GlobalTimeDilation.Dilation, 1.0f, 0.001f);
	TestEqual(TEXT("Fully slowed in the middle"), EvaluateStep(SlowMo, MakeContext(0.5f, 1.0f)).GlobalTimeDilation.Dilation, 0.3f, 0.001f);
	TestEqual(TEXT("Normal speed at the end"), EvaluateStep(SlowMo, MakeContext(1.0f, 1.0f)).GlobalTimeDilation.Dilation, 1.0f, 0.001f);

	const float Easing = EvaluateStep(SlowMo, MakeContext(0.1f, 1.0f)).GlobalTimeDilation.Dilation;
	TestTrue(TEXT("Partway slowed while ramping in"), Easing < 1.0f && Easing > 0.3f);

	TestEqual(TEXT("Intensity blends toward normal speed"), EvaluateStep(SlowMo, MakeContext(0.5f, 1.0f, 0.5f)).GlobalTimeDilation.Dilation, 0.65f, 0.001f);
	TestFalse(TEXT("Slow-mo cannot preview"), SlowMo->SupportsPreview());
	TestTrue(TEXT("Slow-mo starts with a flat curve"), SlowMo->UsesConstantIntensityByDefault());

	UFeelStep_GlobalHitstop* Hitstop = NewObject<UFeelStep_GlobalHitstop>(GetTransientPackage());
	TestTrue(TEXT("Hitstops start with a flat curve"), Hitstop->UsesConstantIntensityByDefault());
	return true;
}

#undef FEEL_TEST_FLAGS

#endif // WITH_DEV_AUTOMATION_TESTS
