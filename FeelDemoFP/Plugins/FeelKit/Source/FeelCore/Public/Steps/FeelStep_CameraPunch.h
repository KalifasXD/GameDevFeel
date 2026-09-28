// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Steps/FeelStep_ShapedMotion.h"
#include "FeelStep_CameraPunch.generated.h"

/** Where a directional step takes its direction from. */
UENUM(BlueprintType)
enum class EFeelDirectionSource : uint8
{
	/** The direction set on the step. */
	StepSettings UMETA(DisplayName = "Step Settings"),
	/** The Direction passed in the play context. Falls back to the step settings when the play passed none. */
	PlayDirection UMETA(DisplayName = "Play Direction"),
	/** Away from the Location passed in the play context, toward the target. Falls back to the step settings without a location. */
	AwayFromPlayLocation UMETA(DisplayName = "Away From Play Location"),
	/** Toward the Location passed in the play context. Falls back to the step settings without a location. */
	TowardPlayLocation UMETA(DisplayName = "Toward Play Location"),
};

/** Punches the camera along a direction with a physical spring, kick or smooth motion. */
UCLASS(meta = (DisplayName = "Camera Punch"))
class FEELCORE_API UFeelStep_CameraPunch : public UFeelStep_ShapedMotion
{
	GENERATED_BODY()

public:
	/** Camera-space location offset at the peak (X forward, Y right, Z up). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Punch")
	FVector LocationPunch = FVector(-10.0, 0.0, -4.0);

	/** Rotation offset at the peak, in degrees. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Punch")
	FRotator RotationPunch = FRotator(-2.5, 0.0, 0.0);

	/**
	 * Where the punch direction comes from. With a play direction or location, the punch keeps the strength of Location
	 * Punch and Rotation Punch but points along that direction, as seen from the camera when the play started.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Punch")
	EFeelDirectionSource DirectionSource = EFeelDirectionSource::StepSettings;

	/** Varies the punch direction per play by up to this angle, so repeated plays point in slightly different directions. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Punch", meta = (ClampMin = "0", ClampMax = "90", Units = "Degrees"))
	float DirectionJitter = 0.0f;

	virtual void Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const override;
	virtual FGameplayTag GetDefaultChannel_Implementation() const override;
};
