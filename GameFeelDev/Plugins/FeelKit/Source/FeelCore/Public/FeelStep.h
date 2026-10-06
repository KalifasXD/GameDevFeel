// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FeelParameters.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "FeelStep.generated.h"

class AActor;
class APlayerController;
class IFeelOutputSink;
class UFeelRecipe;
class USceneComponent;
class UWorld;

/** Inputs for computing a step's output at one moment. A step's output must depend only on these values. */
USTRUCT(BlueprintType)
struct FEELCORE_API FFeelStepEvalContext
{
	GENERATED_BODY()

	/** Seconds since the track started. */
	UPROPERTY(BlueprintReadOnly, Category = "Feel")
	float LocalTime = 0.0f;

	/** Track length in seconds. */
	UPROPERTY(BlueprintReadOnly, Category = "Feel")
	float Duration = 0.0f;

	/** Normalized track time: 0 at the start, 1 at the end. */
	UPROPERTY(BlueprintReadOnly, Category = "Feel")
	float Alpha = 0.0f;

	/** Final intensity: call intensity x recipe default intensity x track intensity curve. */
	UPROPERTY(BlueprintReadOnly, Category = "Feel")
	float Intensity = 1.0f;

	/** Seed for noise and randomness, so the same moment always produces the same output. */
	UPROPERTY(BlueprintReadOnly, Category = "Feel")
	int32 Seed = 0;

	/** The play context's Direction, normalized, in world space. Zero when the play passed none. */
	UPROPERTY(BlueprintReadOnly, Category = "Feel")
	FVector Direction = FVector::ZeroVector;

	/** The play context's Direction in the view space of the target's local player when the play started (X forward, Y right, Z up). Zero when none. */
	UPROPERTY(BlueprintReadOnly, Category = "Feel")
	FVector ViewDirection = FVector::ZeroVector;

	/** Direction from the play context's Location to the target, in the same view space. Zero when the play passed no location. */
	UPROPERTY(BlueprintReadOnly, Category = "Feel")
	FVector ViewDirectionFromLocation = FVector::ZeroVector;
};

/** Runtime information for side-effect callbacks (OnStart, OnStop). */
USTRUCT(BlueprintType)
struct FEELCORE_API FFeelContext
{
	GENERATED_BODY()

	/** World the recipe plays in. */
	UPROPERTY(BlueprintReadOnly, Category = "Feel")
	TObjectPtr<UWorld> World = nullptr;

	/** Actor this track plays on, if any: the play's target, or its instigator for tracks that apply to the instigator. */
	UPROPERTY(BlueprintReadOnly, Category = "Feel")
	TObjectPtr<AActor> Target = nullptr;

	/** The second actor passed in the play context as Instigator, if any. */
	UPROPERTY(BlueprintReadOnly, Category = "Feel")
	TObjectPtr<AActor> Instigator = nullptr;

	/** Information passed with the play: parameter values (defaults filled in), direction, location, normal and context tags. */
	UPROPERTY(BlueprintReadOnly, Category = "Feel")
	FFeelPlayContext PlayContext;

	/** Local player controller that owns the effect, if any. */
	UPROPERTY(BlueprintReadOnly, Category = "Feel")
	TObjectPtr<APlayerController> PlayerController = nullptr;

	/** Recipe being played. */
	UPROPERTY(BlueprintReadOnly, Category = "Feel")
	TObjectPtr<UFeelRecipe> Recipe = nullptr;

	/** Unscaled seconds since the recipe started. */
	UPROPERTY(BlueprintReadOnly, Category = "Feel")
	float ElapsedRealTime = 0.0f;

	/** Scene component the recipe targets, if any (the root component for actor targets). */
	UPROPERTY(BlueprintReadOnly, Category = "Feel")
	TObjectPtr<USceneComponent> TargetComponent = nullptr;

	/** World location of the target: the location, component, actor or player camera it points at. */
	UPROPERTY(BlueprintReadOnly, Category = "Feel")
	FVector TargetLocation = FVector::ZeroVector;

	/** Intensity of this track when it started: call intensity x recipe default x intensity curve x comfort. */
	UPROPERTY(BlueprintReadOnly, Category = "Feel")
	float Intensity = 1.0f;

	/** Length of this track in seconds. 0 for instant tracks. */
	UPROPERTY(BlueprintReadOnly, Category = "Feel")
	float TrackDuration = 0.0f;

	/** Identifies the playing instance within World. */
	UPROPERTY(BlueprintReadOnly, Category = "Feel")
	int32 InstanceId = 0;

	/**
	 * Index of this track in the recipe. Tracks of recipes played by Play Recipe tracks get numbers from 10000 up, and
	 * tracks of a release recipe negative numbers, so every track of a play has a number of its own.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Feel")
	int32 TrackIndex = INDEX_NONE;
};

/**
 * A single effect placed on a recipe track.
 *
 * Continuous output comes from Evaluate, which must be a pure function of its context: no state carried between
 * frames. That is what lets the editor scrub in any direction and match gameplay exactly.
 * OnStart and OnStop are only for side effects such as sounds or events.
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class FEELCORE_API UFeelStep : public UObject
{
	GENERATED_BODY()

public:
	/** Computes this step's contribution for one moment and sends it to the sink. */
	virtual void Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const;

	/** Called when the track begins. Use for side effects only. */
	UFUNCTION(BlueprintNativeEvent, Category = "Feel")
	void OnStart(const FFeelContext& Context);

	/** Called when the track ends or is canceled. */
	UFUNCTION(BlueprintNativeEvent, Category = "Feel")
	void OnStop(const FFeelContext& Context, bool bInterrupted);

	/** Channel assigned to a new track that uses this step. */
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Feel")
	FGameplayTag GetDefaultChannel() const;

	/** Whether the editor preview can simulate this step. */
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Feel")
	bool SupportsPreview() const;

	/** Whether evaluating out of order is safe. Steps that keep state between frames must return false. */
	virtual bool SupportsScrub() const { return true; }

	/** True for steps that shape their own envelope; new tracks then start with a flat intensity curve instead of a fade. */
	virtual bool UsesConstantIntensityByDefault() const { return false; }

	/** False for steps that work as instant tracks (length 0), such as sounds and events. Used by data validation. */
	virtual bool RequiresDuration() const { return true; }

#if WITH_EDITOR
	/** Adds problems with this step's settings; reported by data validation on save. */
	virtual void ValidateStep(TArray<FText>& OutErrors, TArray<FText>& OutWarnings) const {}
#endif
};
