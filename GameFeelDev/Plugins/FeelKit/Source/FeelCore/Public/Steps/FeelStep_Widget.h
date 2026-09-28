// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Steps/FeelStep_ShapedMotion.h"
#include "FeelStep_Widget.generated.h"

/** Scales, moves and turns a widget target and springs it back, for buttons, counters and icons. Needs a widget target. */
UCLASS(meta = (DisplayName = "Widget Punch"))
class FEELCORE_API UFeelStep_WidgetPunch : public UFeelStep_ShapedMotion
{
	GENERATED_BODY()

public:
	/** Scale change at the peak: 0.2 makes the widget 20% larger. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Punch")
	FVector2D ScaleChange = FVector2D(0.2, 0.2);

	/** Movement at the peak, in slate units. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Punch")
	FVector2D Translation = FVector2D(0.0, 0.0);

	/** Rotation at the peak. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Punch", meta = (Units = "Degrees"))
	float AngleDegrees = 0.0f;

	virtual void Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const override;
	virtual FGameplayTag GetDefaultChannel_Implementation() const override;
	virtual bool SupportsPreview_Implementation() const override { return false; }
};

/** Shakes a widget target, fading out over the track. Needs a widget target. */
UCLASS(meta = (DisplayName = "Widget Shake"))
class FEELCORE_API UFeelStep_WidgetShake : public UFeelStep
{
	GENERATED_BODY()

public:
	/** Largest movement at the start, in slate units. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake")
	FVector2D Amplitude = FVector2D(8.0, 3.0);

	/** Largest rotation at the start. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake", meta = (Units = "Degrees"))
	float AngleAmplitude = 0.0f;

	/** Shake speed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake", meta = (ClampMin = "1", ClampMax = "80", Units = "Hertz"))
	float Frequency = 30.0f;

	/** How quickly the shake dies down: 0 keeps full strength, 1 fades linearly, higher fades faster. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake", meta = (ClampMin = "0", ClampMax = "8"))
	float Decay = 1.0f;

	virtual void Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const override;
	virtual FGameplayTag GetDefaultChannel_Implementation() const override;
	virtual bool UsesConstantIntensityByDefault() const override { return true; }
	virtual bool SupportsPreview_Implementation() const override { return false; }
};

/**
 * Tints a widget target toward a color and back. Applies to user widgets, images and text blocks (color and
 * opacity) and borders (brush color). Needs a widget target.
 */
UCLASS(meta = (DisplayName = "Widget Flash"))
class FEELCORE_API UFeelStep_WidgetFlash : public UFeelStep_ShapedMotion
{
	GENERATED_BODY()

public:
	UFeelStep_WidgetFlash();

	/** Color at the peak. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flash")
	FLinearColor Color = FLinearColor(1.0f, 0.2f, 0.2f);

	/** How far the widget color moves toward Color at the peak. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flash", meta = (ClampMin = "0", ClampMax = "1"))
	float Strength = 1.0f;

	virtual void Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const override;
	virtual FGameplayTag GetDefaultChannel_Implementation() const override;
	virtual bool SupportsPreview_Implementation() const override { return false; }
};
