// Copyright 2026 Billo. All Rights Reserved.

#include "Steps/FeelStep_SlowMoRamp.h"

#include "FeelOutputSink.h"
#include "FeelTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelStep_SlowMoRamp)

void UFeelStep_SlowMoRamp::Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const
{
	const float RampIn = RampInTime > 0.0f ? Context.LocalTime / RampInTime : 1.0f;
	const float RampOut = RampOutTime > 0.0f ? (Context.Duration - Context.LocalTime) / RampOutTime : 1.0f;
	const float Ramp = FMath::SmoothStep(0.0f, 1.0f, FMath::Clamp(FMath::Min(RampIn, RampOut), 0.0f, 1.0f));
	const float Weight = Ramp * FMath::Clamp(Context.Intensity, 0.0f, 1.0f);

	Sink.AddTimeDilation(EFeelTimeScope::Global, FMath::Lerp(1.0f, TimeDilation, Weight), Priority);
}

FGameplayTag UFeelStep_SlowMoRamp::GetDefaultChannel_Implementation() const
{
	return FeelTags::Time_SlowMo;
}

bool UFeelStep_SlowMoRamp::SupportsPreview_Implementation() const
{
	return false;
}
