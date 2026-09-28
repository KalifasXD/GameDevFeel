// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FeelMotion.h"
#include "FeelStep.h"
#include "FeelStep_ShapedMotion.generated.h"

/** Base for steps that move along a motion shape (spring, kick or smooth). */
UCLASS(Abstract)
class FEELCORE_API UFeelStep_ShapedMotion : public UFeelStep
{
	GENERATED_BODY()

public:
	/** How the motion moves over the track. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
	EFeelMotionShape Shape = EFeelMotionShape::Spring;

	/** Spring oscillations per second. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion", meta = (ClampMin = "0.1", Units = "Hertz", EditCondition = "Shape == EFeelMotionShape::Spring", EditConditionHides))
	float Frequency = 5.0f;

	/** How quickly the spring settles. Higher values bounce less. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion", meta = (ClampMin = "0", EditCondition = "Shape == EFeelMotionShape::Spring", EditConditionHides))
	float Damping = 7.0f;

	/** Portion of the track spent rising to the peak. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion", meta = (ClampMin = "0.01", ClampMax = "0.99", EditCondition = "Shape == EFeelMotionShape::Kick", EditConditionHides))
	float AttackFraction = 0.15f;

	/** Plays the shape this many times across the track, for heartbeats, double hits and flurries. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion", meta = (ClampMin = "1", ClampMax = "32"))
	int32 Repeats = 1;

	virtual bool UsesConstantIntensityByDefault() const override { return true; }

protected:
	/** Motion value at this moment before intensity. Peaks at 1. */
	float EvaluateMotion(const FFeelStepEvalContext& Context) const;
};
