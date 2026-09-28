// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FeelMotion.h"
#include "Steps/FeelStep_ShapedMotion.h"
#include "FeelStep_ActorExtras.generated.h"

/**
 * Shakes and tilts the target back and forth, fading out over the track. Works on moving targets: the wobble is
 * applied on top of wherever the target is.
 */
UCLASS(meta = (DisplayName = "Mesh Wobble"))
class FEELCORE_API UFeelStep_MeshWobble : public UFeelStep
{
	GENERATED_BODY()

public:
	/** Largest tilt around each local axis at the start, in degrees. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wobble")
	FRotator TiltAmplitude = FRotator(0.0, 0.0, 8.0);

	/** Largest movement along each local axis at the start, in centimeters. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wobble")
	FVector MoveAmplitude = FVector(0.0, 0.0, 0.0);

	/** Wobbles per second. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wobble", meta = (ClampMin = "0.1", ClampMax = "60", Units = "Hertz"))
	float Frequency = 10.0f;

	/** How quickly the wobble dies down over the track. 0 keeps full strength to the end; 1 fades linearly; higher fades faster. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wobble", meta = (ClampMin = "0", ClampMax = "8"))
	float Decay = 1.5f;

	/** Irregular noise motion instead of a regular back-and-forth swing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wobble")
	bool bNoise = false;

	virtual void Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const override;
	virtual FGameplayTag GetDefaultChannel_Implementation() const override;
	virtual bool UsesConstantIntensityByDefault() const override { return true; }
};

/** Brightens and optionally recolors the target's lights, with an optional flicker. */
UCLASS(meta = (DisplayName = "Light Flash"))
class FEELCORE_API UFeelStep_LightFlash : public UFeelStep_ShapedMotion
{
	GENERATED_BODY()

public:
	UFeelStep_LightFlash();

	/**
	 * Extra brightness at the peak, as a multiple of the light's own intensity: 2 makes it three times as bright, -1 turns
	 * it off. Applies to the target when it is a light component, otherwise to every light component of the target actor.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light", meta = (ClampMin = "-1", ClampMax = "50"))
	float IntensityChange = 2.0f;

	/** Color the light moves toward. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light")
	FLinearColor Color = FLinearColor::White;

	/** How far the light color moves toward Color at the peak. 0 keeps the light's color. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light", meta = (ClampMin = "0", ClampMax = "1"))
	float ColorStrength = 0.0f;

	/** Rapidly switch between bright and normal while the track plays, like an unstable light. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker")
	bool bFlicker = false;

	/** Flicker changes per second. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker", meta = (ClampMin = "1", ClampMax = "60", Units = "Hertz", EditCondition = "bFlicker"))
	float FlickerRate = 18.0f;

	virtual void Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const override;
	virtual FGameplayTag GetDefaultChannel_Implementation() const override;
	virtual bool SupportsPreview_Implementation() const override { return false; }
};
