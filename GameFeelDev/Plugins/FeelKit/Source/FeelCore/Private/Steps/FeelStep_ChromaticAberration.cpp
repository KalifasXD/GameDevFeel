// Copyright 2026 Billo. All Rights Reserved.

#include "Steps/FeelStep_ChromaticAberration.h"

#include "FeelOutputSink.h"
#include "FeelTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelStep_ChromaticAberration)

UFeelStep_ChromaticAberration::UFeelStep_ChromaticAberration()
{
	Shape = EFeelMotionShape::Kick;
}

void UFeelStep_ChromaticAberration::Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const
{
	const float Weight = FMath::Clamp(EvaluateMotion(Context) * Context.Intensity, 0.0f, 1.0f);
	Sink.AddPostProcess(EFeelPostProcessParameter::ChromaticAberration, FringeIntensity, Weight);
}

FGameplayTag UFeelStep_ChromaticAberration::GetDefaultChannel_Implementation() const
{
	return FeelTags::Screen_Distortion;
}
