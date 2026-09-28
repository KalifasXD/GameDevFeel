// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Steps/FeelStep_ShapedMotion.h"
#include "FeelStep_CameraExtras.generated.h"

/** Rolls the camera around its view axis and back. Players can turn camera roll off in their comfort settings. */
UCLASS(meta = (DisplayName = "Camera Roll"))
class FEELCORE_API UFeelStep_CameraRoll : public UFeelStep_ShapedMotion
{
	GENERATED_BODY()

public:
	/** Roll angle at the peak. Positive rolls clockwise. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Roll", meta = (ClampMin = "-45", ClampMax = "45", Units = "Degrees"))
	float RollDegrees = 4.0f;

	/** Pick clockwise or counterclockwise at random for each play. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Roll")
	bool bRandomDirection = true;

	virtual void Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const override;
	virtual FGameplayTag GetDefaultChannel_Implementation() const override;
};

/** Narrows or widens the field of view, holds, then returns, for zoom-ins and focus moments. */
UCLASS(meta = (DisplayName = "Camera Zoom"))
class FEELCORE_API UFeelStep_CameraZoom : public UFeelStep
{
	GENERATED_BODY()

public:
	/** Field of view change while held. Negative zooms in, positive zooms out. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom", meta = (ClampMin = "-60", ClampMax = "60", Units = "Degrees"))
	float FieldOfViewChange = -10.0f;

	/** Portion of the track spent easing into the zoom. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom", meta = (ClampMin = "0", ClampMax = "1"))
	float EaseInFraction = 0.25f;

	/** Portion of the track spent easing back out. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom", meta = (ClampMin = "0", ClampMax = "1"))
	float EaseOutFraction = 0.35f;

	virtual void Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const override;
	virtual FGameplayTag GetDefaultChannel_Implementation() const override;
	virtual bool UsesConstantIntensityByDefault() const override { return true; }
};

/**
 * Turns the camera briefly toward the play context's Location, or along its Direction when no location was passed, then
 * returns. Does nothing when the play passed neither.
 */
UCLASS(meta = (DisplayName = "Look-at Nudge"))
class FEELCORE_API UFeelStep_LookAtNudge : public UFeelStep_ShapedMotion
{
	GENERATED_BODY()

public:
	UFeelStep_LookAtNudge();

	/** Portion of the angle to the location that the camera turns at the peak. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nudge", meta = (ClampMin = "0", ClampMax = "1"))
	float TurnFraction = 0.2f;

	/** Largest turn at the peak, per axis. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nudge", meta = (ClampMin = "0", ClampMax = "45", Units = "Degrees"))
	float MaxTurnDegrees = 6.0f;

	virtual void Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const override;
	virtual FGameplayTag GetDefaultChannel_Implementation() const override;
};
