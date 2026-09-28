// Copyright 2026 Billo. All Rights Reserved.

#include "FeelComfortTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelComfortTypes)

float FFeelComfortScales::GetGroupScale(EFeelComfortGroup Group) const
{
	switch (Group)
	{
	case EFeelComfortGroup::CameraShake:
		return CameraShake;
	case EFeelComfortGroup::CameraMotion:
		return CameraMotion;
	case EFeelComfortGroup::Flashes:
		return Flashes;
	case EFeelComfortGroup::HitstopAndSlowMo:
		return HitstopAndSlowMo;
	case EFeelComfortGroup::ScreenDistortion:
		return ScreenDistortion;
	case EFeelComfortGroup::Haptics:
		return Haptics;
	case EFeelComfortGroup::None:
	default:
		return 1.0f;
	}
}

void FFeelComfortScales::SetGroupScale(EFeelComfortGroup Group, float Scale)
{
	const float Clamped = FMath::Clamp(Scale, 0.0f, 1.0f);
	switch (Group)
	{
	case EFeelComfortGroup::CameraShake:
		CameraShake = Clamped;
		break;
	case EFeelComfortGroup::CameraMotion:
		CameraMotion = Clamped;
		break;
	case EFeelComfortGroup::Flashes:
		Flashes = Clamped;
		break;
	case EFeelComfortGroup::HitstopAndSlowMo:
		HitstopAndSlowMo = Clamped;
		break;
	case EFeelComfortGroup::ScreenDistortion:
		ScreenDistortion = Clamped;
		break;
	case EFeelComfortGroup::Haptics:
		Haptics = Clamped;
		break;
	case EFeelComfortGroup::None:
	default:
		break;
	}
}

void FFeelComfortScales::ClampScales()
{
	for (float* Scale : { &Master, &CameraShake, &CameraMotion, &Flashes, &HitstopAndSlowMo, &ScreenDistortion, &Haptics, &SoftenedFlashScale })
	{
		*Scale = FMath::Clamp(*Scale, 0.0f, 1.0f);
	}
	MaxFlashesPerSecond = FMath::Clamp(MaxFlashesPerSecond, 0.1f, 30.0f);
	MaxFieldOfViewChangePerSecond = FMath::Max(MaxFieldOfViewChangePerSecond, 0.0f);
}

bool FFeelComfortScales::operator==(const FFeelComfortScales& Other) const
{
	return Master == Other.Master
		&& CameraShake == Other.CameraShake
		&& CameraMotion == Other.CameraMotion
		&& Flashes == Other.Flashes
		&& HitstopAndSlowMo == Other.HitstopAndSlowMo
		&& ScreenDistortion == Other.ScreenDistortion
		&& Haptics == Other.Haptics
		&& bLimitFlashes == Other.bLimitFlashes
		&& MaxFlashesPerSecond == Other.MaxFlashesPerSecond
		&& FlashLimitMode == Other.FlashLimitMode
		&& SoftenedFlashScale == Other.SoftenedFlashScale
		&& bAllowCameraRoll == Other.bAllowCameraRoll
		&& MaxFieldOfViewChangePerSecond == Other.MaxFieldOfViewChangePerSecond;
}

float FFeelFlashLimiter::RegisterFlash(double Now, const FFeelComfortScales& Scales)
{
	if (!Scales.bLimitFlashes)
	{
		return 1.0f;
	}

	// Rates below one per second use a longer window with one flash allowed.
	const double Rate = FMath::Max(Scales.MaxFlashesPerSecond, 0.1f);
	const double Window = FMath::Max(1.0, 1.0 / Rate);
	const int32 Allowed = FMath::Max(1, FMath::FloorToInt32(Rate * Window + 0.001));

	RecentFlashes.RemoveAll([Now, Window](double Started) { return Now - Started >= Window; });
	if (RecentFlashes.Num() < Allowed)
	{
		RecentFlashes.Add(Now);
		return 1.0f;
	}

	if (Scales.FlashLimitMode == EFeelFlashLimitMode::Suppress)
	{
		// Suppressed flashes are not shown, so they do not count.
		return 0.0f;
	}
	RecentFlashes.Add(Now);
	return FMath::Clamp(Scales.SoftenedFlashScale, 0.0f, 1.0f);
}

namespace FeelComfort
{
	EFeelComfortGroup FindGroup(const FGameplayTag& Channel, TConstArrayView<FFeelChannelComfortMapping> Mappings)
	{
		EFeelComfortGroup Result = EFeelComfortGroup::None;
		if (!Channel.IsValid())
		{
			return Result;
		}

		// A matching parent tag is always a prefix of the channel, so the longest matching name is the most specific.
		int32 BestLength = -1;
		for (const FFeelChannelComfortMapping& Mapping : Mappings)
		{
			if (Mapping.Channel.IsValid() && Channel.MatchesTag(Mapping.Channel))
			{
				const int32 Length = Mapping.Channel.GetTagName().GetStringLength();
				if (Length > BestLength)
				{
					BestLength = Length;
					Result = Mapping.Group;
				}
			}
		}
		return Result;
	}

	bool IsMotionGroup(EFeelComfortGroup Group)
	{
		return Group == EFeelComfortGroup::CameraShake || Group == EFeelComfortGroup::CameraMotion;
	}

	void ApplyMotionComfort(const FFeelComfortScales& Scales, FRotator& InOutRotationOffset, float& InOutFieldOfViewOffset, float& InOutPreviousFieldOfView, float DeltaSeconds)
	{
		if (!Scales.bAllowCameraRoll)
		{
			InOutRotationOffset.Roll = 0.0;
		}

		if (Scales.MaxFieldOfViewChangePerSecond > 0.0f && DeltaSeconds > 0.0f)
		{
			const float MaxStep = Scales.MaxFieldOfViewChangePerSecond * DeltaSeconds;
			InOutFieldOfViewOffset = FMath::Clamp(InOutFieldOfViewOffset, InOutPreviousFieldOfView - MaxStep, InOutPreviousFieldOfView + MaxStep);
		}
		InOutPreviousFieldOfView = InOutFieldOfViewOffset;
	}
}

float FFeelComfortContext::GetScale(const FGameplayTag& Channel) const
{
	if (!Scales)
	{
		return 1.0f;
	}
	return Scales->Master * Scales->GetGroupScale(GetGroup(Channel));
}

EFeelComfortGroup FFeelComfortContext::GetGroup(const FGameplayTag& Channel) const
{
	return FeelComfort::FindGroup(Channel, Mappings);
}
