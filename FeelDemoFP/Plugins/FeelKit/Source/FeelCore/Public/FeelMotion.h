// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FeelMotion.generated.h"

/** Shape of a punch-like motion over a track. */
UENUM(BlueprintType)
enum class EFeelMotionShape : uint8
{
	/** Springy overshoot that settles, like a physical hit. */
	Spring,
	/** Fast rise to the peak, then an eased return. */
	Kick,
	/** Smooth rise and fall. */
	Smooth,
};

/** A local axis of the target. */
UENUM(BlueprintType)
enum class EFeelAxis : uint8
{
	X,
	Y,
	Z,
};

/** Motion curves shared by steps. Every function depends only on its inputs, so scrubbing stays exact. */
namespace FeelMotion
{
	/** Damped spring after an impulse: 0 at the start, first peak exactly 1, then a decaying oscillation around 0. */
	FEELCORE_API float Spring(float Time, float Frequency, float Damping);

	/** Time of the spring's first peak. */
	FEELCORE_API float SpringPeakTime(float Frequency, float Damping);

	/** Rises to 1 at AttackFraction with ease-out, then returns to 0 at Alpha 1 with ease-in-out. */
	FEELCORE_API float Kick(float Alpha, float AttackFraction);

	/** Half sine: 0 at both ends, 1 in the middle. */
	FEELCORE_API float Smooth(float Alpha);

	/** Eases up over InFraction of the track, holds at 1, eases down over OutFraction. Fractions are clamped so they never overlap. */
	FEELCORE_API float Envelope(float Alpha, float InFraction, float OutFraction);

	/** Smooth value noise in [-1, 1] from time and a seed. */
	FEELCORE_API float Noise(float Time, int32 Seed);
}
