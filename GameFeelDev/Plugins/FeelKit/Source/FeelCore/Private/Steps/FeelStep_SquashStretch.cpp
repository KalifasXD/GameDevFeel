// Copyright 2026 Billo. All Rights Reserved.

#include "Steps/FeelStep_SquashStretch.h"

#include "FeelOutputSink.h"
#include "FeelTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelStep_SquashStretch)

void UFeelStep_SquashStretch::Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const
{
	const float AxisScale = FMath::Max(1.0f + Amount * EvaluateMotion(Context) * Context.Intensity, 0.05f);
	const float OtherScale = bPreserveVolume ? 1.0f / FMath::Sqrt(AxisScale) : 1.0f;

	FVector ScaleDelta(OtherScale - 1.0f);
	switch (Axis)
	{
	case EFeelAxis::X:
		ScaleDelta.X = AxisScale - 1.0f;
		break;
	case EFeelAxis::Y:
		ScaleDelta.Y = AxisScale - 1.0f;
		break;
	case EFeelAxis::Z:
	default:
		ScaleDelta.Z = AxisScale - 1.0f;
		break;
	}

	Sink.AddTargetScale(ScaleDelta);
}

FGameplayTag UFeelStep_SquashStretch::GetDefaultChannel_Implementation() const
{
	return FeelTags::Actor_Transform;
}
