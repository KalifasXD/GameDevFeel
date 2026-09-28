// Copyright 2026 Billo. All Rights Reserved.

#include "Steps/FeelStep_PlaySound.h"

#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "FeelTags.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelStep_PlaySound)

void UFeelStep_PlaySound::OnStart_Implementation(const FFeelContext& Context)
{
	if (!Sound || !Context.World)
	{
		return;
	}

	const float IntensityScale = bScaleVolumeWithIntensity ? FMath::Clamp(Context.Intensity, 0.0f, 4.0f) : 1.0f;
	const float Volume = VolumeMultiplier * IntensityScale * (1.0f + FMath::FRandRange(-VolumeVariation, VolumeVariation));
	const float Pitch = PitchMultiplier * (1.0f + FMath::FRandRange(-PitchVariation, PitchVariation));

	UAudioComponent* AudioComponent = nullptr;
	if (Placement == EFeelSoundPlacement::AttachedToTarget && Context.TargetComponent)
	{
		AudioComponent = UGameplayStatics::SpawnSoundAttached(Sound, Context.TargetComponent, AttachSocketName, FVector::ZeroVector,
			EAttachLocation::SnapToTarget, true, Volume, Pitch, SoundStartTime, AttenuationSettings, ConcurrencySettings);
	}
	else if (Placement != EFeelSoundPlacement::TwoD)
	{
		AudioComponent = UGameplayStatics::SpawnSoundAtLocation(Context.World, Sound, Context.TargetLocation, FRotator::ZeroRotator,
			Volume, Pitch, SoundStartTime, AttenuationSettings, ConcurrencySettings);
	}
	else
	{
		AudioComponent = UGameplayStatics::SpawnSound2D(Context.World, Sound, Volume, Pitch, SoundStartTime, ConcurrencySettings);
	}

	if (AudioComponent && Context.TrackDuration > 0.0f && (bStopAtTrackEnd || bStopWhenRecipeStops))
	{
		ActiveSounds.Add(MakeSoundKey(Context), AudioComponent);
	}
}

void UFeelStep_PlaySound::OnStop_Implementation(const FFeelContext& Context, bool bInterrupted)
{
	TWeakObjectPtr<UAudioComponent> WeakComponent;
	if (!ActiveSounds.RemoveAndCopyValue(MakeSoundKey(Context), WeakComponent))
	{
		return;
	}

	const bool bShouldStop = bInterrupted ? bStopWhenRecipeStops : bStopAtTrackEnd;
	UAudioComponent* AudioComponent = WeakComponent.Get();
	if (bShouldStop && AudioComponent && AudioComponent->IsPlaying())
	{
		AudioComponent->FadeOut(FadeOutTime, 0.0f);
	}
}

#if WITH_EDITOR
void UFeelStep_PlaySound::ValidateStep(TArray<FText>& OutErrors, TArray<FText>& OutWarnings) const
{
	if (!Sound)
	{
		OutErrors.Add(NSLOCTEXT("FeelKit", "PlaySoundMissingSound", "Play Sound has no sound."));
	}
}
#endif

FGameplayTag UFeelStep_PlaySound::GetDefaultChannel_Implementation() const
{
	return FeelTags::Audio;
}

UFeelStep_PlaySound::FSoundKey UFeelStep_PlaySound::MakeSoundKey(const FFeelContext& Context)
{
	return FSoundKey(FObjectKey(Context.World), Context.InstanceId, Context.TrackIndex);
}
