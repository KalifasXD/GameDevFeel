// Copyright 2026 Billo. All Rights Reserved.

#include "Steps/FeelStep_ScreenFlash.h"

#include "FeelOutputSink.h"
#include "FeelTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelStep_ScreenFlash)

void UFeelStep_ScreenFlash::Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const
{
	Sink.AddScreenFlash(Color, FMath::Clamp(MaxOpacity * Context.Intensity, 0.0f, 1.0f));
}

FGameplayTag UFeelStep_ScreenFlash::GetDefaultChannel_Implementation() const
{
	return FeelTags::Screen_Flash;
}
