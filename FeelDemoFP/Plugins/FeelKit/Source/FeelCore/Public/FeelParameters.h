// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Curves/CurveFloat.h"
#include "GameplayTagContainer.h"
#include "FeelParameters.generated.h"

class AActor;

/**
 * A named number a recipe accepts from the game each time it plays. Tracks can use it to scale their intensity, so one
 * recipe covers a range of strengths instead of needing a copy per strength. The recipe editor shows a slider for each
 * parameter to preview the whole range.
 */
USTRUCT(BlueprintType)
struct FEELCORE_API FFeelRecipeParameter
{
	GENERATED_BODY()

	/**
	 * Name the game uses to pass a value, and tracks use to read it.
	 * A parameter named Distance is special: when the game does not pass it, it is filled in with the distance in
	 * centimeters from the play's target to the nearest local player camera.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	FName Name;

	/** Value used when the game does not pass one. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	float DefaultValue = 0.0f;

	/** Lowest meaningful value. Lower values are clamped to it. Track mappings read this as 0. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	float MinValue = 0.0f;

	/** Highest meaningful value. Higher values are clamped to it. Track mappings read this as 1. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	float MaxValue = 1.0f;

	/**
	 * Optional accumulator (Project Settings > Plugins > FeelKit > Accumulators) this parameter reads when the game does not pass a
	 * value. It is read every frame while the recipe plays: the value stored for the play's target actor, else the value of
	 * the play's instigator, else the global value. Leave empty to use the default value.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter", meta = (GetOptions = "GetAccumulatorNames"))
	FName Accumulator;

	/** Explains what the value represents, for whoever sets it. Shown as the tooltip of the preview slider. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	FText Description;

	/** Value clamped to the range, then mapped to 0 (min) to 1 (max). */
	float Normalize(float Value) const;
};

/** Makes a track's intensity follow one of the recipe's parameters. */
USTRUCT(BlueprintType)
struct FEELCORE_API FFeelParameterMapping
{
	GENERATED_BODY()

	FFeelParameterMapping();

	/** Recipe parameter this mapping reads. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mapping", meta = (GetOptions = "GetParameterNames"))
	FName Parameter;

	/**
	 * How the parameter turns into an intensity multiplier. Horizontal axis: the parameter from its min (0) to its max (1).
	 * Vertical axis: the multiplier applied to the track. The default straight line from 0 to 1 means intensity grows in
	 * proportion to the parameter. An empty curve behaves like that line.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mapping")
	FRuntimeFloatCurve Curve;

	/** Multiplier for a normalized parameter value (0 to 1). */
	float Evaluate(float NormalizedValue) const;
};

/** Which of the play's two actors a track's actor effects (scale, materials, actor time, sounds attached to an actor) apply to. */
UENUM(BlueprintType)
enum class EFeelTrackTarget : uint8
{
	/** The actor or component the recipe was played on. */
	PlayTarget UMETA(DisplayName = "Play Target"),
	/** The second actor passed in the play context as Instigator. The track is skipped when the play has none. */
	Instigator,
};

/**
 * Extra information passed with a play. Everything is optional; an empty context plays the recipe exactly like Play Feel.
 * It carries values for the recipe's parameters, an optional second actor that tracks can apply to, and details about
 * what triggered the play that steps and Blueprint events can read.
 */
USTRUCT(BlueprintType)
struct FEELCORE_API FFeelPlayContext
{
	GENERATED_BODY()

	/** Values for the recipe's parameters, by parameter name. Parameters not listed use their default value. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel")
	TMap<FName, float> Parameters;

	/**
	 * A second actor involved in this play, besides the target: whatever caused or started it. Tracks whose Applies To is
	 * Instigator play their actor effects on this actor instead of the target. Leave empty when only the target matters.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel")
	TObjectPtr<AActor> Instigator = nullptr;

	/** A world-space direction associated with the play, if any. Zero means none. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel")
	FVector Direction = FVector::ZeroVector;

	/** A world-space location associated with the play, when it differs from the target's location. Zero means none. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel")
	FVector Location = FVector::ZeroVector;

	/** A world-space surface normal associated with the play, if any. Zero means none. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel")
	FVector Normal = FVector::ZeroVector;

	/** Gameplay tags describing the circumstances of the play. Feel Maps use them to choose between recipe variants. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel")
	FGameplayTagContainer ContextTags;
};
