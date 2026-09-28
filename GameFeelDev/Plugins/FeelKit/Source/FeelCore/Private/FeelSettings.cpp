// Copyright 2026 Billo. All Rights Reserved.

#include "FeelSettings.h"

#include "FeelTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelSettings)

UFeelSettings::UFeelSettings()
{
	ReducedMotionPreset.CameraShake = 0.25f;
	ReducedMotionPreset.CameraMotion = 0.25f;
	ReducedMotionPreset.ScreenDistortion = 0.5f;
	ReducedMotionPreset.bAllowCameraRoll = false;
	ReducedMotionPreset.MaxFieldOfViewChangePerSecond = 40.0f;

	// Stricter limiter.
	ReducedFlashingPreset.Flashes = 0.2f;
	ReducedFlashingPreset.MaxFlashesPerSecond = 1.0f;
	ReducedFlashingPreset.FlashLimitMode = EFeelFlashLimitMode::Suppress;

	NoHapticsPreset.Haptics = 0.0f;

	ComfortMenuClass = FSoftClassPath(TEXT("/FeelKit/UI/WBP_FeelComfortMenu.WBP_FeelComfortMenu_C"));

	ChannelComfortGroups = {
		FFeelChannelComfortMapping(FeelTags::Camera_Shake, EFeelComfortGroup::CameraShake),
		FFeelChannelComfortMapping(FeelTags::Camera_Motion, EFeelComfortGroup::CameraMotion),
		FFeelChannelComfortMapping(FeelTags::Screen_Flash, EFeelComfortGroup::Flashes),
		FFeelChannelComfortMapping(FeelTags::Screen_Distortion, EFeelComfortGroup::ScreenDistortion),
		FFeelChannelComfortMapping(FeelTags::Screen_Color, EFeelComfortGroup::ScreenDistortion),
		FFeelChannelComfortMapping(FeelTags::Actor_Light, EFeelComfortGroup::Flashes),
		FFeelChannelComfortMapping(FeelTags::Time_Hitstop, EFeelComfortGroup::HitstopAndSlowMo),
		FFeelChannelComfortMapping(FeelTags::Time_SlowMo, EFeelComfortGroup::HitstopAndSlowMo),
		FFeelChannelComfortMapping(FeelTags::Haptics, EFeelComfortGroup::Haptics),
	};
}

FFeelComfortScales UFeelSettings::GetPresetScales(EFeelBuiltInComfortPreset Preset) const
{
	switch (Preset)
	{
	case EFeelBuiltInComfortPreset::ReducedMotion:
		return ReducedMotionPreset;
	case EFeelBuiltInComfortPreset::ReducedFlashing:
		return ReducedFlashingPreset;
	case EFeelBuiltInComfortPreset::NoHaptics:
		return NoHapticsPreset;
	case EFeelBuiltInComfortPreset::Default:
	default:
		return DefaultComfortScales;
	}
}

const FFeelAccumulatorDefinition* UFeelSettings::FindAccumulator(FName AccumulatorName) const
{
	if (AccumulatorName.IsNone())
	{
		return nullptr;
	}
	return Accumulators.FindByPredicate([AccumulatorName](const FFeelAccumulatorDefinition& Definition) { return Definition.Name == AccumulatorName; });
}
