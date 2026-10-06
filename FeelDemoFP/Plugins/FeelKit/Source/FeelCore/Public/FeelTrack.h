// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Curves/CurveFloat.h"
#include "FeelParameters.h"
#include "GameplayTagContainer.h"
#include "Math/Interval.h"
#include "FeelTrack.generated.h"

class UFeelStep;

/** How a track depends on the way a sustained play was released. */
UENUM()
enum class EFeelReleaseCondition : uint8
{
	/** The track plays however the play is released. */
	Any,
	/** Only when the recipe's Release Parameter reached Release At, which released the play: the payoff of a full charge. */
	WhenReleaseParameterReached UMETA(DisplayName = "When Release Parameter Reached"),
	/** Only when the play was released before its Release Parameter reached Release At, such as a charge let go early. */
	WhenReleasedEarly UMETA(DisplayName = "When Released Early"),
};

/** Conditions that decide whether a track plays for a given instance. */
USTRUCT(BlueprintType)
struct FEELCORE_API FFeelConditions
{
	GENERATED_BODY()

	/** Only play when the target belongs to a local player. */
	UPROPERTY(EditAnywhere, Category = "Conditions")
	bool bLocalPlayerOnly = false;

	/** Skip when the target is farther than this from the local camera. 0 means unlimited. */
	UPROPERTY(EditAnywhere, Category = "Conditions", meta = (ClampMin = "0", Units = "Centimeters"))
	float MaxDistance = 0.0f;

	/**
	 * Ties the track to how a sustained play was released. Meant for tracks in the recipe's ending, which starts at Sustain
	 * End: a full charge and a charge let go early can end differently. A track that starts before the play is released
	 * is skipped, because the release is not known yet.
	 */
	UPROPERTY(EditAnywhere, Category = "Conditions")
	EFeelReleaseCondition Release = EFeelReleaseCondition::Any;

	/** Probability that the track plays. */
	UPROPERTY(EditAnywhere, Category = "Conditions", meta = (ClampMin = "0", ClampMax = "1"))
	float Chance = 1.0f;

	/** Platforms the track plays on. Empty means all platforms. */
	UPROPERTY(EditAnywhere, Category = "Conditions")
	TArray<FName> Platforms;
};

/** One timed step inside a recipe. */
USTRUCT(BlueprintType)
struct FEELCORE_API FFeelTrack
{
	GENERATED_BODY()

	FFeelTrack();

	/** The effect this track plays. */
	UPROPERTY(EditAnywhere, Instanced, Category = "Track")
	TObjectPtr<UFeelStep> Step;

	/** Seconds from recipe start. */
	UPROPERTY(EditAnywhere, Category = "Track", meta = (ClampMin = "0", Units = "Seconds"))
	float StartTime = 0.0f;

	/** Length in seconds. 0 means an instant step. */
	UPROPERTY(EditAnywhere, Category = "Track", meta = (ClampMin = "0", Units = "Seconds"))
	float Duration = 0.5f;

	/** Channel of this track, e.g. Feel.Camera.Shake. */
	UPROPERTY(EditAnywhere, Category = "Track", meta = (Categories = "Feel"))
	FGameplayTag Channel;

	/** Intensity over normalized track time (0 to 1). An empty curve means constant 1. */
	UPROPERTY(EditAnywhere, Category = "Track")
	FRuntimeFloatCurve IntensityCurve;

	/** Seed for noise and randomness in the step. */
	UPROPERTY(EditAnywhere, Category = "Track")
	int32 Seed = 0;

	/**
	 * Which actor this track's actor effects (scale, materials, actor time, attached sounds, Blueprint events) apply to:
	 * the play's target, or the Instigator actor passed in the play context. Camera, screen and haptic effects go to the
	 * local player of that actor.
	 */
	UPROPERTY(EditAnywhere, Category = "Track")
	EFeelTrackTarget AppliesTo = EFeelTrackTarget::PlayTarget;

	/** Recipe parameters that scale this track's intensity. Each mapping multiplies the intensity; they combine with the intensity curve. */
	UPROPERTY(EditAnywhere, Category = "Parameter Mappings", meta = (TitleProperty = "Parameter"))
	TArray<FFeelParameterMapping> ParameterMappings;

	/** Intensity multiplier picked at random between Min and Max for each play. 1 to 1 means no variation. */
	UPROPERTY(EditAnywhere, Category = "Randomness")
	FFloatInterval RandomIntensity = FFloatInterval(1.0f, 1.0f);

	/** Length multiplier picked at random between Min and Max for each play. 1 to 1 means no variation. */
	UPROPERTY(EditAnywhere, Category = "Randomness")
	FFloatInterval RandomDurationScale = FFloatInterval(1.0f, 1.0f);

	/** Editor mute. Disabled tracks produce no output. */
	UPROPERTY(EditAnywhere, Category = "Track")
	bool bEnabled = true;

	/** Marks the track as carrying gameplay-critical information. */
	UPROPERTY(EditAnywhere, Category = "Comfort")
	bool bEssential = false;

	/** Played instead when comfort settings disable this channel and the track is essential. */
	UPROPERTY(EditAnywhere, Instanced, Category = "Comfort")
	TObjectPtr<UFeelStep> SubstituteStep;

	/** Minimum intensity scale when the track is essential and has no substitute. */
	UPROPERTY(EditAnywhere, Category = "Comfort", meta = (ClampMin = "0", ClampMax = "1"))
	float EssentialFloor = 0.0f;

	/** Conditions for playing this track. */
	UPROPERTY(EditAnywhere, Category = "Conditions")
	FFeelConditions Conditions;

#if WITH_EDITORONLY_DATA
	/** Editor solo. When any track is soloed, the preview plays only soloed tracks. */
	UPROPERTY()
	bool bSolo = false;
#endif

	float GetEndTime() const { return StartTime + FMath::Max(Duration, 0.0f); }

	/** Evaluates the intensity curve at normalized time Alpha. */
	float EvaluateIntensityCurve(float Alpha) const;
};
