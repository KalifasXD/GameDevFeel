// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/Scene.h"
#include "FeelArbiters.h"
#include "FeelFrameOutput.h"
#include "Steps/FeelStep_ChromaticAberration.h"
#include "Steps/FeelStep_VignettePulse.h"
#include "UObject/Package.h"

#define FEEL_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

namespace FeelPostProcessTests
{
	FFeelStepEvalContext MakeContext(float LocalTime, float Duration, float Intensity = 1.0f)
	{
		FFeelStepEvalContext Context;
		Context.LocalTime = LocalTime;
		Context.Duration = Duration;
		Context.Alpha = Duration > 0.0f ? FMath::Clamp(LocalTime / Duration, 0.0f, 1.0f) : 0.0f;
		Context.Intensity = Intensity;
		return Context;
	}

	FFeelFrameOutput EvaluateStep(const UFeelStep* Step, const FFeelStepEvalContext& Context)
	{
		FFeelOutputAccumulator Accumulator;
		Step->Evaluate(Context, Accumulator);
		return Accumulator.Output;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelPostProcessStepsTest, "FeelKit.Steps.PostProcessPulses", FEEL_TEST_FLAGS)
bool FFeelPostProcessStepsTest::RunTest(const FString& Parameters)
{
	using namespace FeelPostProcessTests;

	UFeelStep_VignettePulse* Vignette = NewObject<UFeelStep_VignettePulse>(GetTransientPackage());
	FFeelFrameOutput Output = EvaluateStep(Vignette, MakeContext(0.5f, 1.0f));
	const FFeelPostProcessContribution& VignetteOut = Output.GetPostProcess(EFeelPostProcessParameter::VignetteIntensity);
	TestEqual(TEXT("Vignette value"), VignetteOut.Value, 1.0f);
	TestEqual(TEXT("Vignette fully blended at the peak"), VignetteOut.Weight, 1.0f, 0.001f);
	TestEqual(TEXT("Intensity scales the vignette blend"), EvaluateStep(Vignette, MakeContext(0.5f, 1.0f, 0.5f)).GetPostProcess(EFeelPostProcessParameter::VignetteIntensity).Weight, 0.5f, 0.001f);
	TestEqual(TEXT("No vignette at the start"), EvaluateStep(Vignette, MakeContext(0.0f, 1.0f)).GetPostProcess(EFeelPostProcessParameter::VignetteIntensity).Weight, 0.0f, 0.001f);

	Vignette->Repeats = 2;
	TestEqual(TEXT("Repeats: first beat peaks at a quarter"), EvaluateStep(Vignette, MakeContext(0.25f, 1.0f)).GetPostProcess(EFeelPostProcessParameter::VignetteIntensity).Weight, 1.0f, 0.001f);
	TestEqual(TEXT("Repeats: second beat peaks at three quarters"), EvaluateStep(Vignette, MakeContext(0.75f, 1.0f)).GetPostProcess(EFeelPostProcessParameter::VignetteIntensity).Weight, 1.0f, 0.001f);
	TestEqual(TEXT("Repeats: quiet between beats"), EvaluateStep(Vignette, MakeContext(0.5f, 1.0f)).GetPostProcess(EFeelPostProcessParameter::VignetteIntensity).Weight, 0.0f, 0.001f);

	UFeelStep_ChromaticAberration* Fringe = NewObject<UFeelStep_ChromaticAberration>(GetTransientPackage());
	Output = EvaluateStep(Fringe, MakeContext(0.15f, 1.0f));
	TestEqual(TEXT("Chromatic aberration value"), Output.GetPostProcess(EFeelPostProcessParameter::ChromaticAberration).Value, 3.0f);
	TestEqual(TEXT("Chromatic aberration peaks at the kick attack"), Output.GetPostProcess(EFeelPostProcessParameter::ChromaticAberration).Weight, 1.0f, 0.001f);
	TestEqual(TEXT("Chromatic aberration leaves the vignette alone"), Output.GetPostProcess(EFeelPostProcessParameter::VignetteIntensity).Weight, 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelPostProcessCombineTest, "FeelKit.Output.PostProcessBlends", FEEL_TEST_FLAGS)
bool FFeelPostProcessCombineTest::RunTest(const FString& Parameters)
{
	FFeelOutputAccumulator Accumulator;
	Accumulator.AddPostProcess(EFeelPostProcessParameter::VignetteIntensity, 0.6f, 0.3f);
	Accumulator.AddPostProcess(EFeelPostProcessParameter::VignetteIntensity, 0.9f, 0.8f);
	Accumulator.AddPostProcess(EFeelPostProcessParameter::VignetteIntensity, 0.2f, 0.5f);
	TestEqual(TEXT("Strongest vignette weight wins"), Accumulator.Output.GetPostProcess(EFeelPostProcessParameter::VignetteIntensity).Weight, 0.8f);
	TestEqual(TEXT("Strongest vignette keeps its value"), Accumulator.Output.GetPostProcess(EFeelPostProcessParameter::VignetteIntensity).Value, 0.9f);

	FFeelFrameOutput A;
	A.PostProcess[static_cast<int32>(EFeelPostProcessParameter::ChromaticAberration)] = { 2.0f, 0.4f };
	FFeelFrameOutput B;
	B.PostProcess[static_cast<int32>(EFeelPostProcessParameter::ChromaticAberration)] = { 4.0f, 0.9f };
	const TArray<const FFeelFrameOutput*> Inputs = { &A, &B };
	FFeelFrameOutput Screen;
	FeelArbiters::ResolveScreen(Inputs, Screen);
	TestEqual(TEXT("Arbiter keeps the strongest chromatic aberration"), Screen.GetPostProcess(EFeelPostProcessParameter::ChromaticAberration).Value, 4.0f);

	int32 Blends = 0;
	bool bVignetteOverridden = false;
	float VignetteWeight = 0.0f;
	Accumulator.Output.ForEachPostProcessBlend([&](FPostProcessSettings& Settings, float Weight)
	{
		++Blends;
		if (Settings.bOverride_VignetteIntensity && !Settings.bOverride_SceneFringeIntensity)
		{
			bVignetteOverridden = FMath::IsNearlyEqual(Settings.VignetteIntensity, 0.9f);
			VignetteWeight = Weight;
		}
	});
	TestEqual(TEXT("One blend per active parameter"), Blends, 1);
	TestTrue(TEXT("Blend overrides only the vignette with its value"), bVignetteOverridden);
	TestEqual(TEXT("Blend uses the contribution weight"), VignetteWeight, 0.8f);

	int32 ScreenBlends = 0;
	Screen.ForEachPostProcessBlend([&](FPostProcessSettings& Settings, float Weight)
	{
		++ScreenBlends;
		TestTrue(TEXT("Chromatic aberration blend overrides fringe intensity"), Settings.bOverride_SceneFringeIntensity && FMath::IsNearlyEqual(Settings.SceneFringeIntensity, 4.0f));
	});
	TestEqual(TEXT("Inactive parameters produce no blend"), ScreenBlends, 1);

	return true;
}

#undef FEEL_TEST_FLAGS

#endif // WITH_DEV_AUTOMATION_TESTS
