// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Steps/FeelStep_ShapedMotion.h"
#include "FeelStep_FOVKick.generated.h"

/** Kicks the field of view for speed bursts or impact zooms. */
UCLASS(meta = (DisplayName = "FOV Kick"))
class FEELCORE_API UFeelStep_FOVKick : public UFeelStep_ShapedMotion
{
	GENERATED_BODY()

public:
	UFeelStep_FOVKick();

	/** Field of view change at the peak. Positive widens (speed), negative narrows (impact zoom). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FOV", meta = (ClampMin = "-60", ClampMax = "60", Units = "Degrees"))
	float FieldOfViewKick = 8.0f;

	virtual void Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const override;
	virtual FGameplayTag GetDefaultChannel_Implementation() const override;
};
