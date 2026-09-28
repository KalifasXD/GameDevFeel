// Copyright 2026 Billo. All Rights Reserved.

#include "Steps/FeelStep_ForceFeedbackCurve.h"

#include "FeelOutputSink.h"
#include "FeelTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelStep_ForceFeedbackCurve)

UFeelStep_ForceFeedbackCurve::UFeelStep_ForceFeedbackCurve()
{
	Shape = EFeelMotionShape::Kick;
}

void UFeelStep_ForceFeedbackCurve::Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const
{
	const float Strength = FMath::Clamp(FMath::Abs(EvaluateMotion(Context)) * Context.Intensity, 0.0f, 1.0f);

	float Ripple = 1.0f;
	if (RippleFrequency > 0.0f)
	{
		const float Dip = 0.5f - 0.5f * FMath::Cos(UE_TWO_PI * RippleFrequency * Context.LocalTime);
		Ripple = 1.0f - RippleDepth * Dip;
	}

	const float Scale = Strength * Ripple;
	FFeelForceFeedbackValues Values;
	Values.LeftLarge = LeftLarge * Scale;
	Values.LeftSmall = LeftSmall * Scale;
	Values.RightLarge = RightLarge * Scale;
	Values.RightSmall = RightSmall * Scale;
	Sink.AddForceFeedback(Values);
}

FGameplayTag UFeelStep_ForceFeedbackCurve::GetDefaultChannel_Implementation() const
{
	return FeelTags::Haptics;
}

#if WITH_EDITOR
void UFeelStep_ForceFeedbackCurve::ValidateStep(TArray<FText>& OutErrors, TArray<FText>& OutWarnings) const
{
	if (LeftLarge <= 0.0f && LeftSmall <= 0.0f && RightLarge <= 0.0f && RightSmall <= 0.0f)
	{
		OutWarnings.Add(NSLOCTEXT("FeelKit", "ForceFeedbackSilent", "Force Feedback Curve has every motor at 0, so it never vibrates."));
	}
}
#endif

bool UFeelStep_ForceFeedbackCurve::SupportsPreview_Implementation() const
{
	return false;
}
