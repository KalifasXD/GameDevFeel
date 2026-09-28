// Copyright 2026 Billo. All Rights Reserved.

#include "Steps/FeelStep_ActorExtras.h"

#include "FeelOutputSink.h"
#include "FeelTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelStep_ActorExtras)

void UFeelStep_MeshWobble::Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const
{
	const float Fade = Decay > 0.0f ? FMath::Pow(1.0f - FMath::Clamp(Context.Alpha, 0.0f, 1.0f), Decay) : 1.0f;
	const float Strength = Fade * Context.Intensity;
	if (FMath::IsNearlyZero(Strength))
	{
		return;
	}

	const float Phase = Context.LocalTime * Frequency;
	auto Wave = [this, Phase, &Context](int32 Channel)
	{
		if (bNoise)
		{
			return FeelMotion::Noise(Phase, Context.Seed + Channel * 7919);
		}
		// Each axis gets its own phase so the motion is not flat.
		return FMath::Sin(2.0f * UE_PI * Phase + static_cast<float>(Channel) * 1.7f);
	};

	const FRotator Tilt(TiltAmplitude.Pitch * Wave(0), TiltAmplitude.Yaw * Wave(1), TiltAmplitude.Roll * Wave(2));
	const FVector Move(MoveAmplitude.X * Wave(3), MoveAmplitude.Y * Wave(4), MoveAmplitude.Z * Wave(5));
	Sink.AddTargetTransform(Move * Strength, Tilt * Strength);
}

FGameplayTag UFeelStep_MeshWobble::GetDefaultChannel_Implementation() const
{
	return FeelTags::Actor_Transform;
}

UFeelStep_LightFlash::UFeelStep_LightFlash()
{
	Shape = EFeelMotionShape::Kick;
}

void UFeelStep_LightFlash::Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const
{
	float Motion = EvaluateMotion(Context) * Context.Intensity;
	if (bFlicker)
	{
		// On or off per flicker slot, chosen from the seed, so scrubbing shows the same pattern.
		const int32 Slot = FMath::FloorToInt32(Context.LocalTime * FlickerRate);
		uint32 Hash = HashCombineFast(GetTypeHash(Context.Seed), GetTypeHash(Slot));
		// Finalizer: neighboring slots get unrelated values.
		Hash ^= Hash >> 16;
		Hash *= 0x7feb352dU;
		Hash ^= Hash >> 15;
		Hash *= 0x846ca68bU;
		Hash ^= Hash >> 16;
		Motion *= (Hash % 100u) < 55u ? 1.0f : 0.0f;
	}
	Sink.AddLight(IntensityChange * Motion, Color, ColorStrength * Motion);
}

FGameplayTag UFeelStep_LightFlash::GetDefaultChannel_Implementation() const
{
	return FeelTags::Actor_Light;
}
