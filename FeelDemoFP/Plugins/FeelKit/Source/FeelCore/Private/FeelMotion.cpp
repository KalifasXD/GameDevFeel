// Copyright 2026 Billo. All Rights Reserved.

#include "FeelMotion.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelMotion)

namespace FeelMotion
{
	namespace Private
	{
		float AngularFrequency(float Frequency)
		{
			return UE_TWO_PI * FMath::Max(Frequency, 0.01f);
		}
	}

	float SpringPeakTime(float Frequency, float Damping)
	{
		// d/dt [e^(-D t) sin(w t)] = 0  =>  tan(w t) = w / D
		const float Omega = Private::AngularFrequency(Frequency);
		return FMath::Atan2(Omega, FMath::Max(Damping, 0.0f)) / Omega;
	}

	float Spring(float Time, float Frequency, float Damping)
	{
		if (Time <= 0.0f)
		{
			return 0.0f;
		}

		const float Omega = Private::AngularFrequency(Frequency);
		const float Decay = FMath::Max(Damping, 0.0f);
		const float PeakTime = SpringPeakTime(Frequency, Damping);
		const float Peak = FMath::Exp(-Decay * PeakTime) * FMath::Sin(Omega * PeakTime);
		return FMath::Exp(-Decay * Time) * FMath::Sin(Omega * Time) / FMath::Max(Peak, UE_KINDA_SMALL_NUMBER);
	}

	float Kick(float Alpha, float AttackFraction)
	{
		const float ClampedAlpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
		const float Attack = FMath::Clamp(AttackFraction, 0.01f, 0.99f);
		if (ClampedAlpha < Attack)
		{
			const float Rise = ClampedAlpha / Attack;
			return 1.0f - (1.0f - Rise) * (1.0f - Rise);
		}

		const float Fall = (ClampedAlpha - Attack) / (1.0f - Attack);
		return 1.0f - Fall * Fall * (3.0f - 2.0f * Fall);
	}

	float Smooth(float Alpha)
	{
		return FMath::Sin(FMath::Clamp(Alpha, 0.0f, 1.0f) * UE_PI);
	}

	float Envelope(float Alpha, float InFraction, float OutFraction)
	{
		const float ClampedAlpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
		float In = FMath::Clamp(InFraction, 0.0f, 1.0f);
		float Out = FMath::Clamp(OutFraction, 0.0f, 1.0f);
		if (In + Out > 1.0f)
		{
			const float Scale = 1.0f / (In + Out);
			In *= Scale;
			Out *= Scale;
		}

		if (In > 0.0f && ClampedAlpha < In)
		{
			return FMath::SmoothStep(0.0f, 1.0f, ClampedAlpha / In);
		}
		if (Out > 0.0f && ClampedAlpha > 1.0f - Out)
		{
			return FMath::SmoothStep(0.0f, 1.0f, (1.0f - ClampedAlpha) / Out);
		}
		return 1.0f;
	}

	float Noise(float Time, int32 Seed)
	{
		// Perlin noise sampled along a seed-dependent line, so different seeds give unrelated motion.
		const float Offset = static_cast<float>(Seed % 10007) * 13.37f;
		return FMath::Clamp(FMath::PerlinNoise1D(Time + Offset) * 2.0f, -1.0f, 1.0f);
	}
}
