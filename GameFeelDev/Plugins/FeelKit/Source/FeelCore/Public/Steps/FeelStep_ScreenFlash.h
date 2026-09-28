// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FeelStep.h"
#include "FeelStep_ScreenFlash.generated.h"

/** Full-screen color flash. Shape the fade with the track intensity curve. */
UCLASS(meta = (DisplayName = "Screen Flash"))
class FEELCORE_API UFeelStep_ScreenFlash : public UFeelStep
{
	GENERATED_BODY()

public:
	/** Flash color. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flash")
	FLinearColor Color = FLinearColor::White;

	/** Opacity at full intensity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flash", meta = (ClampMin = "0", ClampMax = "1"))
	float MaxOpacity = 0.6f;

	virtual void Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const override;
	virtual FGameplayTag GetDefaultChannel_Implementation() const override;
};
