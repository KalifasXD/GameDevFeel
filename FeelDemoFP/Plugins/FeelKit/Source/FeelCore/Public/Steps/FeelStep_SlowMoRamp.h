// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FeelStep.h"
#include "FeelStep_SlowMoRamp.generated.h"

/** Eases the world into slow motion and back out. Recipe timing stays in real time. */
UCLASS(meta = (DisplayName = "Slow-mo Ramp"))
class FEELCORE_API UFeelStep_SlowMoRamp : public UFeelStep
{
	GENERATED_BODY()

public:
	/** Time dilation while fully slowed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slow-mo", meta = (ClampMin = "0.0001", ClampMax = "1"))
	float TimeDilation = 0.3f;

	/** Real seconds to ease into slow motion at the start of the track. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slow-mo", meta = (ClampMin = "0", Units = "Seconds"))
	float RampInTime = 0.15f;

	/** Real seconds to ease back to normal speed before the track ends. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slow-mo", meta = (ClampMin = "0", Units = "Seconds"))
	float RampOutTime = 0.3f;

	/** When slow motions and hitstops overlap, the highest priority wins; equal priorities keep the slowest. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slow-mo")
	int32 Priority = 0;

	virtual void Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const override;
	virtual FGameplayTag GetDefaultChannel_Implementation() const override;
	virtual bool SupportsPreview_Implementation() const override;
	virtual bool UsesConstantIntensityByDefault() const override { return true; }
};
