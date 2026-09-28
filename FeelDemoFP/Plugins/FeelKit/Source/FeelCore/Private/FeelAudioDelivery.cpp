// Copyright 2026 Billo. All Rights Reserved.

#include "FeelAudioDelivery.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "UObject/Package.h"

namespace FeelAudioDeliveryPrivate
{
	constexpr float Levels[] = { 20000.0f, 8000.0f, 4000.0f, 2000.0f, 1000.0f, 500.0f, 250.0f };
	constexpr float OverrideFadeSeconds = 0.03f;
	constexpr float LowPassFadeSeconds = 0.08f;
	constexpr float ChangeThreshold = 0.005f;
}

FFeelAudioDelivery::~FFeelAudioDelivery()
{
	RestoreAll();
}

TConstArrayView<float> FFeelAudioDelivery::GetLowPassLevels()
{
	return FeelAudioDeliveryPrivate::Levels;
}

int32 FFeelAudioDelivery::FindLowPassLevel(float Frequency)
{
	const TConstArrayView<float> Levels = GetLowPassLevels();
	const float LogFrequency = FMath::Loge(FMath::Clamp(Frequency, 20.0f, 20000.0f));
	int32 Best = 0;
	float BestDistance = TNumericLimits<float>::Max();
	for (int32 Index = 0; Index < Levels.Num(); ++Index)
	{
		const float Distance = FMath::Abs(FMath::Loge(Levels[Index]) - LogFrequency);
		if (Distance < BestDistance)
		{
			BestDistance = Distance;
			Best = Index;
		}
	}
	return Best;
}

void FFeelAudioDelivery::Apply(UWorld* InWorld, TConstArrayView<FFeelSoundClassAdjust> Adjusts)
{
	using namespace FeelAudioDeliveryPrivate;

	if (World.Get() != InWorld)
	{
		RestoreAll();
		World = InWorld;
	}
	if (!InWorld)
	{
		return;
	}

	// Volume and pitch through one override mix.
	bool bAnyOverride = false;
	for (const FFeelSoundClassAdjust& Adjust : Adjusts)
	{
		if (!Adjust.SoundClass || (FMath::IsNearlyEqual(Adjust.Volume, 1.0f) && FMath::IsNearlyEqual(Adjust.Pitch, 1.0f)))
		{
			continue;
		}
		bAnyOverride = true;

		if (!OverrideMix.IsValid())
		{
			OverrideMix = TStrongObjectPtr<USoundMix>(NewObject<USoundMix>(GetTransientPackage(), NAME_None, RF_Transient));
		}
		if (!bOverrideMixPushed)
		{
			UGameplayStatics::PushSoundMixModifier(InWorld, OverrideMix.Get());
			bOverrideMixPushed = true;
		}

		FClassOverride* Existing = Overrides.FindByPredicate([&Adjust](const FClassOverride& Override) { return Override.SoundClass.Get() == Adjust.SoundClass; });
		if (!Existing || FMath::Abs(Existing->Volume - Adjust.Volume) > ChangeThreshold || FMath::Abs(Existing->Pitch - Adjust.Pitch) > ChangeThreshold)
		{
			UGameplayStatics::SetSoundMixClassOverride(InWorld, OverrideMix.Get(), Adjust.SoundClass, FMath::Max(Adjust.Volume, 0.0f), FMath::Max(Adjust.Pitch, 0.01f), OverrideFadeSeconds, true);
			if (!Existing)
			{
				Existing = &Overrides.AddDefaulted_GetRef();
				Existing->SoundClass = Adjust.SoundClass;
			}
			Existing->Volume = Adjust.Volume;
			Existing->Pitch = Adjust.Pitch;
		}
	}

	for (auto It = Overrides.CreateIterator(); It; ++It)
	{
		const USoundClass* SoundClass = It->SoundClass.Get();
		const bool bStillRequested = SoundClass && Adjusts.ContainsByPredicate([SoundClass](const FFeelSoundClassAdjust& Adjust)
		{
			return Adjust.SoundClass == SoundClass && !(FMath::IsNearlyEqual(Adjust.Volume, 1.0f) && FMath::IsNearlyEqual(Adjust.Pitch, 1.0f));
		});
		if (!bStillRequested)
		{
			if (SoundClass && OverrideMix.IsValid())
			{
				UGameplayStatics::ClearSoundMixClassOverride(InWorld, OverrideMix.Get(), It->SoundClass.Get(), OverrideFadeSeconds);
			}
			It.RemoveCurrent();
		}
	}

	if (!bAnyOverride && bOverrideMixPushed && Overrides.Num() == 0)
	{
		UGameplayStatics::PopSoundMixModifier(InWorld, OverrideMix.Get());
		bOverrideMixPushed = false;
	}

	// Low-pass: the strongest requested cutoff, snapped to a level.
	USoundClass* LowPassTarget = nullptr;
	float LowestCutoff = 20000.0f;
	for (const FFeelSoundClassAdjust& Adjust : Adjusts)
	{
		if (Adjust.SoundClass && Adjust.LowPassFrequency < LowestCutoff)
		{
			LowestCutoff = Adjust.LowPassFrequency;
			LowPassTarget = Adjust.SoundClass;
		}
	}
	const int32 Level = LowPassTarget ? FindLowPassLevel(LowestCutoff) : 0;
	SetLowPassLevel(Level == 0 ? INDEX_NONE : Level, LowPassTarget);
}

void FFeelAudioDelivery::SetLowPassLevel(int32 Level, USoundClass* SoundClass)
{
	UWorld* CurrentWorld = World.Get();
	if (Level == ActiveLowPassLevel && (Level == INDEX_NONE || LowPassClass.Get() == SoundClass))
	{
		return;
	}

	if (ActiveLowPassLevel != INDEX_NONE && CurrentWorld && LowPassMixes.IsValidIndex(ActiveLowPassLevel) && LowPassMixes[ActiveLowPassLevel].IsValid())
	{
		UGameplayStatics::PopSoundMixModifier(CurrentWorld, LowPassMixes[ActiveLowPassLevel].Get());
	}
	ActiveLowPassLevel = INDEX_NONE;

	if (Level == INDEX_NONE || !SoundClass || !CurrentWorld)
	{
		return;
	}

	const TConstArrayView<float> Levels = GetLowPassLevels();
	if (LowPassMixes.Num() != Levels.Num())
	{
		LowPassMixes.SetNum(Levels.Num());
	}

	// Mixes are rebuilt when the filtered sound class changes.
	if (!LowPassMixes[Level].IsValid() || LowPassClass.Get() != SoundClass)
	{
		USoundMix* Mix = NewObject<USoundMix>(GetTransientPackage(), NAME_None, RF_Transient);
		Mix->FadeInTime = FeelAudioDeliveryPrivate::LowPassFadeSeconds;
		Mix->FadeOutTime = FeelAudioDeliveryPrivate::LowPassFadeSeconds;
		FSoundClassAdjuster Adjuster;
		Adjuster.SoundClassObject = SoundClass;
		Adjuster.LowPassFilterFrequency = Levels[Level];
		Adjuster.bApplyToChildren = true;
		Mix->SoundClassEffects.Add(Adjuster);
		LowPassMixes[Level] = TStrongObjectPtr<USoundMix>(Mix);
	}
	LowPassClass = SoundClass;

	UGameplayStatics::PushSoundMixModifier(CurrentWorld, LowPassMixes[Level].Get());
	ActiveLowPassLevel = Level;
}

void FFeelAudioDelivery::RestoreAll()
{
	UWorld* CurrentWorld = World.Get();
	if (CurrentWorld && OverrideMix.IsValid())
	{
		for (const FClassOverride& Override : Overrides)
		{
			if (USoundClass* SoundClass = Override.SoundClass.Get())
			{
				UGameplayStatics::ClearSoundMixClassOverride(CurrentWorld, OverrideMix.Get(), SoundClass, 0.0f);
			}
		}
		if (bOverrideMixPushed)
		{
			UGameplayStatics::PopSoundMixModifier(CurrentWorld, OverrideMix.Get());
		}
	}
	Overrides.Reset();
	bOverrideMixPushed = false;
	SetLowPassLevel(INDEX_NONE, nullptr);
	ActiveLowPassLevel = INDEX_NONE;
}
