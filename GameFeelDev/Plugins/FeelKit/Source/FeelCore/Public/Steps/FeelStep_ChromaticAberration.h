// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Steps/FeelStep_ShapedMotion.h"
#include "FeelStep_ChromaticAberration.generated.h"

/** Splits the screen's colors for impacts, explosions and disorientation. */
UCLASS(meta = (DisplayName = "Chromatic Aberration"))
class FEELCORE_API UFeelStep_ChromaticAberration : public UFeelStep_ShapedMotion
{
	GENERATED_BODY()

public:
	UFeelStep_ChromaticAberration();

	/** Chromatic aberration intensity at the peak, blended over the scene's own setting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chromatic Aberration", meta = (ClampMin = "0", ClampMax = "5"))
	float FringeIntensity = 3.0f;

	virtual void Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const override;
	virtual FGameplayTag GetDefaultChannel_Implementation() const override;
};
