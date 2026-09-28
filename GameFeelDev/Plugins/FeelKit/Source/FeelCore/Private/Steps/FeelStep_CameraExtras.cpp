// Copyright 2026 Billo. All Rights Reserved.

#include "Steps/FeelStep_CameraExtras.h"

#include "FeelMotion.h"
#include "FeelOutputSink.h"
#include "FeelTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelStep_CameraExtras)

void UFeelStep_CameraRoll::Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const
{
	const float Side = bRandomDirection && (static_cast<uint32>(Context.Seed) & 1u) ? -1.0f : 1.0f;
	const float Roll = RollDegrees * Side * EvaluateMotion(Context) * Context.Intensity;
	Sink.AddCameraOffset(FVector::ZeroVector, FRotator(0.0, 0.0, Roll));
}

FGameplayTag UFeelStep_CameraRoll::GetDefaultChannel_Implementation() const
{
	return FeelTags::Camera_Motion;
}

void UFeelStep_CameraZoom::Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const
{
	Sink.AddFieldOfViewOffset(FieldOfViewChange * FeelMotion::Envelope(Context.Alpha, EaseInFraction, EaseOutFraction) * Context.Intensity);
}

FGameplayTag UFeelStep_CameraZoom::GetDefaultChannel_Implementation() const
{
	return FeelTags::Camera_Motion;
}

UFeelStep_LookAtNudge::UFeelStep_LookAtNudge()
{
	Shape = EFeelMotionShape::Kick;
}

void UFeelStep_LookAtNudge::Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const
{
	// Toward the location: the reverse of the direction from the location to the target.
	FVector Toward = -Context.ViewDirectionFromLocation;
	if (Toward.IsNearlyZero())
	{
		Toward = Context.ViewDirection;
	}
	if (Toward.IsNearlyZero())
	{
		return;
	}

	const FVector Direction = Toward.GetSafeNormal();
	const float YawToTarget = FMath::RadiansToDegrees(FMath::Atan2(Direction.Y, Direction.X));
	const float PitchToTarget = FMath::RadiansToDegrees(FMath::Atan2(Direction.Z, FMath::Sqrt(Direction.X * Direction.X + Direction.Y * Direction.Y)));
	const float Motion = EvaluateMotion(Context) * Context.Intensity;

	const float Yaw = FMath::Clamp(YawToTarget * TurnFraction, -MaxTurnDegrees, MaxTurnDegrees) * Motion;
	const float Pitch = FMath::Clamp(PitchToTarget * TurnFraction, -MaxTurnDegrees, MaxTurnDegrees) * Motion;
	Sink.AddCameraOffset(FVector::ZeroVector, FRotator(Pitch, Yaw, 0.0));
}

FGameplayTag UFeelStep_LookAtNudge::GetDefaultChannel_Implementation() const
{
	return FeelTags::Camera_Motion;
}
