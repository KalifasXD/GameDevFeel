// Copyright 2026 Billo. All Rights Reserved.

#include "Steps/FeelStep_HitFlash.h"

#include "FeelOutputSink.h"
#include "FeelTags.h"
#include "Materials/MaterialInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelStep_HitFlash)

UFeelStep_HitFlash::UFeelStep_HitFlash()
{
	Shape = EFeelMotionShape::Smooth;
}

void UFeelStep_HitFlash::Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const
{
	const float Amount = FMath::Clamp(EvaluateMotion(Context) * Context.Intensity, 0.0f, 1.0f);
	if (FlashMaterial && Amount > 0.0f)
	{
		Sink.AddOverlayFlash(FlashMaterial, Color, Amount);
	}
}

FGameplayTag UFeelStep_HitFlash::GetDefaultChannel_Implementation() const
{
	return FeelTags::Actor_Material;
}

#if WITH_EDITOR
void UFeelStep_HitFlash::ValidateStep(TArray<FText>& OutErrors, TArray<FText>& OutWarnings) const
{
	if (!FlashMaterial)
	{
		OutWarnings.Add(NSLOCTEXT("FeelKit", "HitFlashNoMaterial", "Hit Flash has no flash material, so it does nothing. Pick a translucent overlay material with FlashColor and FlashAmount parameters."));
	}
}
#endif
