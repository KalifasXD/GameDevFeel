// Copyright 2026 Billo. All Rights Reserved.

#include "Steps/FeelStep_Hitstop.h"

#include "FeelOutputSink.h"
#include "FeelTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelStep_Hitstop)

FGameplayTag UFeelStep_HitstopBase::GetDefaultChannel_Implementation() const
{
	return FeelTags::Time_Hitstop;
}

bool UFeelStep_HitstopBase::SupportsPreview_Implementation() const
{
	return false;
}

float UFeelStep_HitstopBase::ComputeDilation(float Intensity) const
{
	return FMath::Lerp(1.0f, TimeDilation, FMath::Clamp(Intensity, 0.0f, 1.0f));
}

void UFeelStep_GlobalHitstop::Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const
{
	Sink.AddTimeDilation(EFeelTimeScope::Global, ComputeDilation(Context.Intensity), Priority);
}

// FEELKIT_PRO_BEGIN
void UFeelStep_ActorHitstop::Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const
{
	Sink.AddTimeDilation(EFeelTimeScope::Target, ComputeDilation(Context.Intensity), Priority);
}
// FEELKIT_PRO_END
