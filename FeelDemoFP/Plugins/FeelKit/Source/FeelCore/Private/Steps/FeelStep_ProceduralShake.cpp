// Copyright 2026 Billo. All Rights Reserved.

#include "Steps/FeelStep_ProceduralShake.h"

#include "FeelOutputSink.h"
#include "FeelTags.h"
#include "Math/RandomStream.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelStep_ProceduralShake)

namespace FeelShake
{
	/** Location X/Y/Z, rotation pitch/yaw/roll, field of view. */
	constexpr int32 NumChannels = 7;
}

void UFeelStep_ProceduralShake::Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const
{
	// Per-channel offsets come from the seed, so the same seed and time always give the same shake.
	FRandomStream Stream(Context.Seed);
	float Offsets[FeelShake::NumChannels];
	for (float& Offset : Offsets)
	{
		Offset = Stream.FRandRange(0.0f, 1000.0f);
	}

	const float Time = Context.LocalTime;
	const float Phase = UE_TWO_PI * Frequency * Time;

	auto Sample = [&](int32 Channel) -> float
	{
		switch (Mode)
		{
		case EFeelShakeMode::Perlin:
			return FMath::PerlinNoise1D(Time * Frequency + Offsets[Channel]);
		case EFeelShakeMode::Sine:
			return FMath::Sin(Phase + Offsets[Channel]);
		case EFeelShakeMode::Directional:
		default:
			return FMath::Sin(Phase);
		}
	};

	const double Scale = Context.Intensity;

	FVector Location;
	if (Mode == EFeelShakeMode::Directional)
	{
		Location = Direction.GetSafeNormal() * (DirectionalAmplitude * Sample(0) * Scale);
	}
	else
	{
		Location = FVector(
			LocationAmplitude.X * Sample(0),
			LocationAmplitude.Y * Sample(1),
			LocationAmplitude.Z * Sample(2)) * Scale;
	}

	const FRotator Rotation(
		RotationAmplitude.Pitch * Sample(3) * Scale,
		RotationAmplitude.Yaw * Sample(4) * Scale,
		RotationAmplitude.Roll * Sample(5) * Scale);

	Sink.AddCameraOffset(Location, Rotation);

	if (FieldOfViewAmplitude != 0.0f)
	{
		Sink.AddFieldOfViewOffset(static_cast<float>(FieldOfViewAmplitude * Sample(6) * Scale));
	}
}

FGameplayTag UFeelStep_ProceduralShake::GetDefaultChannel_Implementation() const
{
	return FeelTags::Camera_Shake;
}
