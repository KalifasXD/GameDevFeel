// Copyright 2026 Billo. All Rights Reserved.

#include "Steps/FeelStep_ScreenColor.h"

#include "FeelMotion.h"
#include "FeelOutputSink.h"
#include "FeelTags.h"
#include "Materials/MaterialInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelStep_ScreenColor)

UFeelStep_Desaturate::UFeelStep_Desaturate()
{
	Shape = EFeelMotionShape::Smooth;
}

void UFeelStep_Desaturate::Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const
{
	const float Weight = FMath::Clamp(EvaluateMotion(Context) * Context.Intensity, 0.0f, 1.0f);
	Sink.AddPostProcess(EFeelPostProcessParameter::Saturation, 1.0f - FMath::Clamp(Amount, 0.0f, 1.0f), Weight);
}

FGameplayTag UFeelStep_Desaturate::GetDefaultChannel_Implementation() const
{
	return FeelTags::Screen_Color;
}

UFeelStep_ColorTint::UFeelStep_ColorTint()
{
	Shape = EFeelMotionShape::Smooth;
}

void UFeelStep_ColorTint::Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const
{
	Sink.AddScreenTint(TintColor, FMath::Clamp(EvaluateMotion(Context) * Context.Intensity * Strength, 0.0f, 1.0f));
}

FGameplayTag UFeelStep_ColorTint::GetDefaultChannel_Implementation() const
{
	return FeelTags::Screen_Color;
}

void UFeelStep_ScreenFade::Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const
{
	const float Alpha = FeelMotion::Envelope(Context.Alpha, FadeInFraction, FadeOutFraction) * MaxOpacity * Context.Intensity;
	Sink.AddScreenFade(FadeColor, FMath::Clamp(Alpha, 0.0f, 1.0f));
}

FGameplayTag UFeelStep_ScreenFade::GetDefaultChannel_Implementation() const
{
	return FeelTags::Screen_Fade;
}

UFeelStep_PostProcessMaterialPulse::UFeelStep_PostProcessMaterialPulse()
{
	Shape = EFeelMotionShape::Smooth;
}

void UFeelStep_PostProcessMaterialPulse::Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const
{
	if (Material)
	{
		Sink.AddPostProcessMaterial(Material, FMath::Clamp(EvaluateMotion(Context) * Context.Intensity * MaxWeight, 0.0f, 1.0f), WeightParameter);
	}
}

FGameplayTag UFeelStep_PostProcessMaterialPulse::GetDefaultChannel_Implementation() const
{
	return FeelTags::Screen_Distortion;
}

#if WITH_EDITOR
void UFeelStep_PostProcessMaterialPulse::ValidateStep(TArray<FText>& OutErrors, TArray<FText>& OutWarnings) const
{
	if (!Material)
	{
		OutErrors.Add(NSLOCTEXT("FeelKit", "PostProcessMaterialMissing", "Post Process Material Pulse has no material."));
	}
}
#endif
