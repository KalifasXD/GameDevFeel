// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Steps/FeelStep_ShapedMotion.h"
#include "FeelStep_VignettePulse.generated.h"

/** Pulses the screen vignette, from a single darkening to a low-health heartbeat with Repeats. */
UCLASS(meta = (DisplayName = "Vignette Pulse"))
class FEELCORE_API UFeelStep_VignettePulse : public UFeelStep_ShapedMotion
{
	GENERATED_BODY()

public:
	UFeelStep_VignettePulse();

	/** Vignette intensity at the peak, blended over the scene's own vignette (the engine default is about 0.4). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vignette", meta = (ClampMin = "0", ClampMax = "1"))
	float VignetteIntensity = 1.0f;

	virtual void Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const override;
	virtual FGameplayTag GetDefaultChannel_Implementation() const override;
};
