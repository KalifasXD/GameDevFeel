// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Steps/FeelStep_ShapedMotion.h"
#include "FeelStep_ForceFeedbackCurve.generated.h"

/** Vibrates the target player's controller with per-motor strengths, a motion shape and optional ripple texture. */
UCLASS(meta = (DisplayName = "Force Feedback Curve"))
class FEELCORE_API UFeelStep_ForceFeedbackCurve : public UFeelStep_ShapedMotion
{
	GENERATED_BODY()

public:
	UFeelStep_ForceFeedbackCurve();

	/** Left heavy motor strength at full intensity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Force Feedback", meta = (ClampMin = "0", ClampMax = "1"))
	float LeftLarge = 1.0f;

	/** Left light motor strength at full intensity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Force Feedback", meta = (ClampMin = "0", ClampMax = "1"))
	float LeftSmall = 0.4f;

	/** Right heavy motor strength at full intensity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Force Feedback", meta = (ClampMin = "0", ClampMax = "1"))
	float RightLarge = 1.0f;

	/** Right light motor strength at full intensity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Force Feedback", meta = (ClampMin = "0", ClampMax = "1"))
	float RightSmall = 0.4f;

	/** Adds texture: the strength ripples this many times per second, like an engine or a heartbeat. 0 is a steady rumble. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Force Feedback", meta = (ClampMin = "0", ClampMax = "60", Units = "Hertz"))
	float RippleFrequency = 0.0f;

	/** How deep the ripple dips, from 0 (no dip) to 1 (fully off at each dip). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Force Feedback", meta = (ClampMin = "0", ClampMax = "1", EditCondition = "RippleFrequency > 0"))
	float RippleDepth = 0.5f;

	virtual void Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const override;
	virtual FGameplayTag GetDefaultChannel_Implementation() const override;

	/** Controller vibration cannot be simulated in the editor preview. */
	virtual bool SupportsPreview_Implementation() const override;

#if WITH_EDITOR
	virtual void ValidateStep(TArray<FText>& OutErrors, TArray<FText>& OutWarnings) const override;
#endif
};
