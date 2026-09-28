// Copyright 2026 Billo. All Rights Reserved.

#include "FeelArbiters.h"

#include "FeelRecipe.h"

namespace FeelArbiters
{
	void ResolveCamera(TConstArrayView<const FFeelFrameOutput*> Inputs, EFeelCameraArbitration Mode, const FFeelCameraCaps& Caps, FFeelFrameOutput& OutResult)
	{
		OutResult.CameraLocationOffset = FVector::ZeroVector;
		OutResult.CameraRotationOffset = FRotator::ZeroRotator;
		OutResult.FieldOfViewOffset = 0.0f;

		if (Mode == EFeelCameraArbitration::AdditiveCapped)
		{
			for (const FFeelFrameOutput* Input : Inputs)
			{
				if (Input)
				{
					OutResult.CameraLocationOffset += Input->CameraLocationOffset;
					OutResult.CameraRotationOffset += Input->CameraRotationOffset;
					OutResult.FieldOfViewOffset += Input->FieldOfViewOffset;
				}
			}

			const double MaxRotation = Caps.MaxRotationOffset;
			OutResult.CameraLocationOffset = OutResult.CameraLocationOffset.GetClampedToMaxSize(Caps.MaxLocationOffset);
			OutResult.CameraRotationOffset = FRotator(
				FMath::Clamp(OutResult.CameraRotationOffset.Pitch, -MaxRotation, MaxRotation),
				FMath::Clamp(OutResult.CameraRotationOffset.Yaw, -MaxRotation, MaxRotation),
				FMath::Clamp(OutResult.CameraRotationOffset.Roll, -MaxRotation, MaxRotation));
			OutResult.FieldOfViewOffset = FMath::Clamp(OutResult.FieldOfViewOffset, -Caps.MaxFieldOfViewOffset, Caps.MaxFieldOfViewOffset);
			return;
		}

		// Strongest wins, separately for location, rotation and field of view. Ties keep the earlier instance.
		double BestLocation = 0.0;
		double BestRotation = 0.0;
		float BestFieldOfView = 0.0f;
		for (const FFeelFrameOutput* Input : Inputs)
		{
			if (!Input)
			{
				continue;
			}

			const double Location = Input->CameraLocationOffset.SizeSquared();
			if (Location > BestLocation)
			{
				BestLocation = Location;
				OutResult.CameraLocationOffset = Input->CameraLocationOffset;
			}

			const double Rotation = Input->CameraRotationOffset.Euler().SizeSquared();
			if (Rotation > BestRotation)
			{
				BestRotation = Rotation;
				OutResult.CameraRotationOffset = Input->CameraRotationOffset;
			}

			const float FieldOfView = FMath::Abs(Input->FieldOfViewOffset);
			if (FieldOfView > BestFieldOfView)
			{
				BestFieldOfView = FieldOfView;
				OutResult.FieldOfViewOffset = Input->FieldOfViewOffset;
			}
		}
	}

	void ResolveScreen(TConstArrayView<const FFeelFrameOutput*> Inputs, FFeelFrameOutput& OutResult)
	{
		OutResult.FlashAlpha = 0.0f;
		OutResult.FlashColor = FLinearColor::White;
		OutResult.TintWeight = 0.0f;
		OutResult.TintColor = FLinearColor::White;
		OutResult.FadeAlpha = 0.0f;
		OutResult.FadeColor = FLinearColor::Black;
		OutResult.PostProcessMaterials.Reset();
		for (FFeelPostProcessContribution& Contribution : OutResult.PostProcess)
		{
			Contribution = FFeelPostProcessContribution();
		}

		for (const FFeelFrameOutput* Input : Inputs)
		{
			if (!Input)
			{
				continue;
			}

			if (Input->FlashAlpha > OutResult.FlashAlpha)
			{
				OutResult.FlashAlpha = Input->FlashAlpha;
				OutResult.FlashColor = Input->FlashColor;
			}
			if (Input->TintWeight > OutResult.TintWeight)
			{
				OutResult.TintWeight = Input->TintWeight;
				OutResult.TintColor = Input->TintColor;
			}
			if (Input->FadeAlpha > OutResult.FadeAlpha)
			{
				OutResult.FadeAlpha = Input->FadeAlpha;
				OutResult.FadeColor = Input->FadeColor;
			}
			for (const FFeelPostProcessMaterial& Material : Input->PostProcessMaterials)
			{
				FFeelFrameOutput::MergePostProcessMaterial(OutResult.PostProcessMaterials, Material);
			}

			for (int32 Index = 0; Index < FFeelFrameOutput::NumPostProcessParameters; ++Index)
			{
				if (Input->PostProcess[Index].Weight > OutResult.PostProcess[Index].Weight)
				{
					OutResult.PostProcess[Index] = Input->PostProcess[Index];
				}
			}
		}
	}
}

void FeelArbiters::ResolveForceFeedback(TConstArrayView<const FFeelFrameOutput*> Inputs, FFeelFrameOutput& OutResult)
{
	OutResult.ForceFeedback = FFeelForceFeedbackValues();
	for (const FFeelFrameOutput* Input : Inputs)
	{
		if (Input)
		{
			OutResult.ForceFeedback.KeepStrongest(Input->ForceFeedback);
		}
	}
}

void FFeelTimeArbiter::Submit(UObject* Target, const FFeelTimeRequest& Request)
{
	if (!Target || !Request.bActive)
	{
		return;
	}

	FPendingRequest& Pending_ = Pending.FindOrAdd(FObjectKey(Target));
	Pending_.Target = Target;
	Pending_.Request.Merge(Request.Dilation, Request.Priority);
}

void FFeelTimeArbiter::Apply(FReadDilation ReadDilation, FWriteDilation WriteDilation)
{
	// Restore clocks nobody requests any more.
	for (auto It = Owned.CreateIterator(); It; ++It)
	{
		if (!Pending.Contains(It.Key()))
		{
			if (UObject* Target = It.Value().Target.Get())
			{
				WriteDilation(Target, It.Value().PreviousDilation);
			}
			It.RemoveCurrent();
		}
	}

	// Apply the winners, remembering the previous dilation the first time a clock is taken over.
	for (const TPair<FObjectKey, FPendingRequest>& Pair : Pending)
	{
		UObject* Target = Pair.Value.Target.Get();
		if (!Target)
		{
			continue;
		}

		if (!Owned.Contains(Pair.Key))
		{
			FOwnedTarget& NewOwned = Owned.Add(Pair.Key);
			NewOwned.Target = Target;
			NewOwned.PreviousDilation = ReadDilation(Target);
		}
		WriteDilation(Target, Pair.Value.Request.Dilation);
	}
	Pending.Reset();

	// Forget destroyed targets.
	for (auto It = Owned.CreateIterator(); It; ++It)
	{
		if (!It.Value().Target.IsValid())
		{
			It.RemoveCurrent();
		}
	}
}

void FFeelTimeArbiter::RestoreAll(FWriteDilation WriteDilation)
{
	for (const TPair<FObjectKey, FOwnedTarget>& Pair : Owned)
	{
		if (UObject* Target = Pair.Value.Target.Get())
		{
			WriteDilation(Target, Pair.Value.PreviousDilation);
		}
	}
	Owned.Reset();
	Pending.Reset();
}

bool FFeelPlaybackGate::CanPlay(const UFeelRecipe& Recipe, const FObjectKey& TargetKey, double NowSeconds, int32 ActiveInstances) const
{
	if (Recipe.MaxConcurrent > 0 && ActiveInstances >= Recipe.MaxConcurrent)
	{
		return false;
	}

	if (Recipe.Cooldown > 0.0f)
	{
		if (const double* LastPlayTime = LastPlayTimes.Find(MakeTuple(FObjectKey(&Recipe), TargetKey)))
		{
			if (NowSeconds - *LastPlayTime < Recipe.Cooldown)
			{
				return false;
			}
		}
	}
	return true;
}

void FFeelPlaybackGate::NotifyPlayed(const UFeelRecipe& Recipe, const FObjectKey& TargetKey, double NowSeconds)
{
	if (Recipe.Cooldown > 0.0f)
	{
		LastPlayTimes.Add(MakeTuple(FObjectKey(&Recipe), TargetKey), NowSeconds);
	}
}
