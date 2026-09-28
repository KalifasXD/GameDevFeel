// Copyright 2026 Billo. All Rights Reserved.

#include "Steps/FeelStep_Widget.h"

#include "FeelMotion.h"
#include "FeelOutputSink.h"
#include "FeelTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelStep_Widget)

void UFeelStep_WidgetPunch::Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const
{
	const float Motion = EvaluateMotion(Context) * Context.Intensity;
	Sink.AddWidgetTransform(Translation * Motion, ScaleChange * Motion, AngleDegrees * Motion);
}

FGameplayTag UFeelStep_WidgetPunch::GetDefaultChannel_Implementation() const
{
	return FeelTags::UI;
}

void UFeelStep_WidgetShake::Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const
{
	const float Fade = Decay > 0.0f ? FMath::Pow(1.0f - FMath::Clamp(Context.Alpha, 0.0f, 1.0f), Decay) : 1.0f;
	const float Strength = Fade * Context.Intensity;
	const float Phase = Context.LocalTime * Frequency;

	const FVector2D Offset(
		Amplitude.X * FeelMotion::Noise(Phase, Context.Seed),
		Amplitude.Y * FeelMotion::Noise(Phase, Context.Seed + 7919));
	const float Angle = AngleAmplitude * FeelMotion::Noise(Phase, Context.Seed + 15838);
	Sink.AddWidgetTransform(Offset * Strength, FVector2D::ZeroVector, Angle * Strength);
}

FGameplayTag UFeelStep_WidgetShake::GetDefaultChannel_Implementation() const
{
	return FeelTags::UI;
}

UFeelStep_WidgetFlash::UFeelStep_WidgetFlash()
{
	Shape = EFeelMotionShape::Kick;
}

void UFeelStep_WidgetFlash::Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const
{
	Sink.AddWidgetColor(Color, FMath::Clamp(EvaluateMotion(Context) * Context.Intensity * Strength, 0.0f, 1.0f));
}

FGameplayTag UFeelStep_WidgetFlash::GetDefaultChannel_Implementation() const
{
	return FeelTags::UI;
}
