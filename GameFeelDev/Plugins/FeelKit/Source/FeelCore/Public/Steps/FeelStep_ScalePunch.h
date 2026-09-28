// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FeelStep.h"
#include "FeelStep_ScalePunch.generated.h"

/** Springy scale punch on the target. The track intensity curve controls the decay. */
UCLASS(meta = (DisplayName = "Scale Punch"))
class FEELCORE_API UFeelStep_ScalePunch : public UFeelStep
{
	GENERATED_BODY()

public:
	/** Relative scale change at the peak, per axis. 0.3 means 30% larger. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Punch")
	FVector Amount = FVector(0.3, 0.3, 0.3);

	/** Extra bounces after the first swing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Punch", meta = (ClampMin = "0", ClampMax = "10"))
	int32 Bounces = 1;

	virtual void Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const override;
	virtual FGameplayTag GetDefaultChannel_Implementation() const override;
};
