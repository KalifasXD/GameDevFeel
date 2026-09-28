// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FeelStep.h"
#include "FeelStep_Hitstop.generated.h"

/** Shared settings of hitstop steps. For a hard stop, give the track a constant intensity curve. */
UCLASS(Abstract)
class FEELCORE_API UFeelStep_HitstopBase : public UFeelStep
{
	GENERATED_BODY()

public:
	/** Time dilation at full intensity. Small values freeze the action. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitstop", meta = (ClampMin = "0.0001", ClampMax = "1"))
	float TimeDilation = 0.05f;

	/** When hitstops on the same clock overlap, the highest priority wins. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitstop")
	int32 Priority = 0;

	virtual FGameplayTag GetDefaultChannel_Implementation() const override;

	/** Time dilation cannot be simulated in the editor preview. */
	virtual bool SupportsPreview_Implementation() const override;

	virtual bool UsesConstantIntensityByDefault() const override { return true; }

protected:
	/** Dilation for the given intensity: 1 at zero intensity, TimeDilation at full intensity. */
	float ComputeDilation(float Intensity) const;
};

/** Slows the whole world. Recipe timing runs in real time, so other tracks keep playing during the stop. */
UCLASS(meta = (DisplayName = "Global Hitstop"))
class FEELCORE_API UFeelStep_GlobalHitstop : public UFeelStep_HitstopBase
{
	GENERATED_BODY()

public:
	virtual void Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const override;
};

// FEELKIT_PRO_BEGIN
/** Slows only the target actor through its custom time dilation. */
UCLASS(meta = (DisplayName = "Actor Hitstop"))
class FEELCORE_API UFeelStep_ActorHitstop : public UFeelStep_HitstopBase
{
	GENERATED_BODY()

public:
	virtual void Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const override;
};
// FEELKIT_PRO_END
