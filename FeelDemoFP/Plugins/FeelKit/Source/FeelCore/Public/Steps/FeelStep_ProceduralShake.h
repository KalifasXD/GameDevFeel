// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FeelStep.h"
#include "FeelStep_ProceduralShake.generated.h"

UENUM(BlueprintType)
enum class EFeelShakeMode : uint8
{
	/** Smooth random motion. */
	Perlin,
	/** Regular oscillation with a random phase per axis. */
	Sine,
	/** Oscillation along a single direction. */
	Directional,
};

/** Camera shake computed from time and seed. */
UCLASS(meta = (DisplayName = "Procedural Shake"))
class FEELCORE_API UFeelStep_ProceduralShake : public UFeelStep
{
	GENERATED_BODY()

public:
	/** Shape of the shake. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake")
	EFeelShakeMode Mode = EFeelShakeMode::Perlin;

	/** Oscillations per second. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake", meta = (ClampMin = "0.01", Units = "Hertz"))
	float Frequency = 12.0f;

	/** Maximum location offset per camera axis (X forward, Y right, Z up). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake", meta = (EditCondition = "Mode != EFeelShakeMode::Directional", EditConditionHides))
	FVector LocationAmplitude = FVector(0.0, 3.0, 3.0);

	/** Camera-space direction of a directional shake. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake", meta = (EditCondition = "Mode == EFeelShakeMode::Directional", EditConditionHides))
	FVector Direction = FVector(0.0, 0.0, 1.0);

	/** Maximum offset along Direction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake", meta = (EditCondition = "Mode == EFeelShakeMode::Directional", EditConditionHides, Units = "Centimeters"))
	float DirectionalAmplitude = 8.0f;

	/** Maximum rotation offset in degrees per axis. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake")
	FRotator RotationAmplitude = FRotator(1.0, 1.0, 0.5);

	/** Maximum field of view change. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake", meta = (Units = "Degrees"))
	float FieldOfViewAmplitude = 0.0f;

	virtual void Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const override;
	virtual FGameplayTag GetDefaultChannel_Implementation() const override;
};
