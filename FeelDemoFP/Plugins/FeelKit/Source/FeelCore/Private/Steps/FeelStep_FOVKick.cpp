// Copyright 2026 Billo. All Rights Reserved.

#include "Steps/FeelStep_FOVKick.h"

#include "FeelOutputSink.h"
#include "FeelTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelStep_FOVKick)

UFeelStep_FOVKick::UFeelStep_FOVKick()
{
	Shape = EFeelMotionShape::Kick;
}

void UFeelStep_FOVKick::Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const
{
	Sink.AddFieldOfViewOffset(FieldOfViewKick * EvaluateMotion(Context) * Context.Intensity);
}

FGameplayTag UFeelStep_FOVKick::GetDefaultChannel_Implementation() const
{
	return FeelTags::Camera_Motion;
}
