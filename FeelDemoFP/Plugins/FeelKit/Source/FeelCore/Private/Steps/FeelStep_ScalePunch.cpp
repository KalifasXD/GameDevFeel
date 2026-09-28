// Copyright 2026 Billo. All Rights Reserved.

#include "Steps/FeelStep_ScalePunch.h"

#include "FeelOutputSink.h"
#include "FeelTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelStep_ScalePunch)

void UFeelStep_ScalePunch::Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const
{
	// Odd number of half waves: starts and ends at the base scale.
	const double HalfWaves = 1.0 + 2.0 * Bounces;
	const double Wave = FMath::Sin(Context.Alpha * UE_DOUBLE_PI * HalfWaves);
	Sink.AddTargetScale(Amount * (Wave * Context.Intensity));
}

FGameplayTag UFeelStep_ScalePunch::GetDefaultChannel_Implementation() const
{
	return FeelTags::Actor_Transform;
}
