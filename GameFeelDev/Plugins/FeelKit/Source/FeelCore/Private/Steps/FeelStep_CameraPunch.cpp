// Copyright 2026 Billo. All Rights Reserved.

#include "Steps/FeelStep_CameraPunch.h"

#include "FeelOutputSink.h"
#include "FeelTags.h"
#include "Math/RandomStream.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelStep_CameraPunch)

void UFeelStep_CameraPunch::Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const
{
	const float Motion = EvaluateMotion(Context) * Context.Intensity;

	FVector Punch = LocationPunch;
	FRotator Rotation = RotationPunch;

	FVector PlayViewDirection = FVector::ZeroVector;
	switch (DirectionSource)
	{
	case EFeelDirectionSource::PlayDirection:
		PlayViewDirection = Context.ViewDirection;
		break;
	case EFeelDirectionSource::AwayFromPlayLocation:
		PlayViewDirection = Context.ViewDirectionFromLocation;
		break;
	case EFeelDirectionSource::TowardPlayLocation:
		PlayViewDirection = -Context.ViewDirectionFromLocation;
		break;
	default:
		break;
	}

	if (!PlayViewDirection.IsNearlyZero())
	{
		// Same strength as the step settings, pointed along the play's direction as seen from the camera.
		const FVector Direction = PlayViewDirection.GetSafeNormal();
		Punch = Direction * LocationPunch.Size();
		const double RotationStrength = FMath::Max3(FMath::Abs(RotationPunch.Pitch), FMath::Abs(RotationPunch.Yaw), FMath::Abs(RotationPunch.Roll));
		Rotation = FRotator(Direction.Z * RotationStrength, Direction.Y * RotationStrength, 0.0);
	}

	if (DirectionJitter > 0.0f)
	{
		// Seeded, so one play keeps its direction for its whole length and scrubbing stays exact.
		const FRandomStream Stream(Context.Seed);
		const FRotator Offset(Stream.FRandRange(-DirectionJitter, DirectionJitter), Stream.FRandRange(-DirectionJitter, DirectionJitter), 0.0f);
		Punch = Offset.RotateVector(Punch);
		Rotation += FRotator(Offset.Pitch * 0.1, Offset.Yaw * 0.1, 0.0);
	}

	Sink.AddCameraOffset(Punch * Motion, Rotation * Motion);
}

FGameplayTag UFeelStep_CameraPunch::GetDefaultChannel_Implementation() const
{
	return FeelTags::Camera_Motion;
}
