// Copyright 2026 Billo. All Rights Reserved.

#include "Steps/FeelStep_ShapedMotion.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelStep_ShapedMotion)

float UFeelStep_ShapedMotion::EvaluateMotion(const FFeelStepEvalContext& Context) const
{
	float LocalTime = Context.LocalTime;
	float Alpha = Context.Alpha;

	// Repeats split the track into equal segments; the last segment ends exactly at the track end.
	const int32 RepeatCount = FMath::Max(Repeats, 1);
	if (RepeatCount > 1 && Context.Duration > 0.0f)
	{
		const float Segment = Context.Duration / static_cast<float>(RepeatCount);
		const int32 SegmentIndex = FMath::Clamp(FMath::FloorToInt32(Context.LocalTime / Segment), 0, RepeatCount - 1);
		LocalTime = Context.LocalTime - static_cast<float>(SegmentIndex) * Segment;
		Alpha = FMath::Clamp(LocalTime / Segment, 0.0f, 1.0f);
	}

	switch (Shape)
	{
	case EFeelMotionShape::Kick:
		return FeelMotion::Kick(Alpha, AttackFraction);
	case EFeelMotionShape::Smooth:
		return FeelMotion::Smooth(Alpha);
	case EFeelMotionShape::Spring:
	default:
		return FeelMotion::Spring(LocalTime, Frequency, Damping);
	}
}
