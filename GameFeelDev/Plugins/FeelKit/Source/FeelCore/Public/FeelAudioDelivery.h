// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FeelOutputSink.h"
#include "Sound/SoundMix.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/WeakObjectPtr.h"

class USoundClass;
class USoundMix;
class UWorld;

/**
 * Applies sound class adjustments (volume, pitch, low-pass) through sound mixes FeelKit creates at runtime, and removes
 * them once nothing requests them. Shared by the runtime subsystem and the editor preview.
 * Volume and pitch follow requests smoothly. The low-pass cutoff moves between a few preset levels.
 */
class FEELCORE_API FFeelAudioDelivery
{
public:
	~FFeelAudioDelivery();

	/** Applies this frame's adjustments in World and removes the ones no longer requested. */
	void Apply(UWorld* World, TConstArrayView<FFeelSoundClassAdjust> Adjusts);

	/** Removes every adjustment immediately. */
	void RestoreAll();

	/** True while any adjustment is applied. */
	bool IsActive() const { return bOverrideMixPushed || ActiveLowPassLevel != INDEX_NONE; }

	/** Cutoff frequencies of the low-pass levels, from none (index 0) to strongest. */
	static TConstArrayView<float> GetLowPassLevels();

	/** Level whose cutoff is closest to Frequency on a logarithmic scale. 0 means no filtering. */
	static int32 FindLowPassLevel(float Frequency);

private:
	struct FClassOverride
	{
		TWeakObjectPtr<USoundClass> SoundClass;
		float Volume = 1.0f;
		float Pitch = 1.0f;
	};

	void SetLowPassLevel(int32 Level, USoundClass* SoundClass);

	TWeakObjectPtr<UWorld> World;
	TStrongObjectPtr<USoundMix> OverrideMix;
	bool bOverrideMixPushed = false;
	TArray<FClassOverride> Overrides;

	TArray<TStrongObjectPtr<USoundMix>> LowPassMixes;
	TWeakObjectPtr<USoundClass> LowPassClass;
	int32 ActiveLowPassLevel = INDEX_NONE;
};
