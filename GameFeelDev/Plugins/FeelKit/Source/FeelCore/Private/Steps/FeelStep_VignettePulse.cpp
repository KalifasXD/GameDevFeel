// Copyright 2026 Billo. All Rights Reserved.

#include "Steps/FeelStep_VignettePulse.h"

#include "FeelOutputSink.h"
#include "FeelTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelStep_VignettePulse)

UFeelStep_VignettePulse::UFeelStep_VignettePulse()
{
	Shape = EFeelMotionShape::Smooth;
}

void UFeelStep_VignettePulse::Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const
{
	const float Weight = FMath::Clamp(EvaluateMotion(Context) * Context.Intensity, 0.0f, 1.0f);
	Sink.AddPostProcess(EFeelPostProcessParameter::VignetteIntensity, VignetteIntensity, Weight);
}

FGameplayTag UFeelStep_VignettePulse::GetDefaultChannel_Implementation() const
{
	return FeelTags::Screen_Distortion;
}
