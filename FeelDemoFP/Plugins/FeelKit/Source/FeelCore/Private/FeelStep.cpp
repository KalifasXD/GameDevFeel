// Copyright 2026 Billo. All Rights Reserved.

#include "FeelStep.h"

#include "FeelOutputSink.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelStep)

void UFeelStep::Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const
{
}

void UFeelStep::OnStart_Implementation(const FFeelContext& Context)
{
}

void UFeelStep::OnStop_Implementation(const FFeelContext& Context, bool bInterrupted)
{
}

FGameplayTag UFeelStep::GetDefaultChannel_Implementation() const
{
	return FGameplayTag();
}

bool UFeelStep::SupportsPreview_Implementation() const
{
	return true;
}
