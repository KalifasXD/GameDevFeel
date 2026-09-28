// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Steps/FeelStep_ShapedMotion.h"
#include "FeelStep_SquashStretch.generated.h"

/** Stretches and squashes the target along one axis, keeping its volume. */
UCLASS(meta = (DisplayName = "Squash and Stretch"))
class FEELCORE_API UFeelStep_SquashStretch : public UFeelStep_ShapedMotion
{
	GENERATED_BODY()

public:
	/** Local axis the target stretches along. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squash and Stretch")
	EFeelAxis Axis = EFeelAxis::Z;

	/** Stretch at the peak. 0.3 makes the target 30% longer along the axis; negative values squash. A spring overshoots into the opposite. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squash and Stretch", meta = (ClampMin = "-0.9", ClampMax = "3"))
	float Amount = 0.3f;

	/** Keeps volume constant: the other axes thin out while stretching and bulge while squashing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squash and Stretch")
	bool bPreserveVolume = true;

	virtual void Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const override;
	virtual FGameplayTag GetDefaultChannel_Implementation() const override;
};
