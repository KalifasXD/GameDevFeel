// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FeelStep.h"
#include "UObject/ObjectKey.h"
#include "UObject/WeakObjectPtr.h"
#include "FeelStep_PlaySound.generated.h"

class UAudioComponent;
class USoundAttenuation;
class USoundBase;
class USoundConcurrency;

/** Where a recipe sound plays. */
UENUM(BlueprintType)
enum class EFeelSoundPlacement : uint8
{
	/** Not positioned in the world, such as UI or player feedback. */
	TwoD UMETA(DisplayName = "2D"),
	/** Follows the target component, optionally at a socket. Falls back to the target location without a component. */
	AttachedToTarget UMETA(DisplayName = "Attached to Target"),
	/** Plays where the target is when the track starts. */
	AtTargetLocation UMETA(DisplayName = "At Target Location"),
};

/** Plays a sound with per-play variation, optionally fading it out with the track. Audible in the editor preview. */
UCLASS(meta = (DisplayName = "Play Sound"))
class FEELCORE_API UFeelStep_PlaySound : public UFeelStep
{
	GENERATED_BODY()

public:
	/** Sound to play. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	TObjectPtr<USoundBase> Sound;

	/** Where the sound plays. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	EFeelSoundPlacement Placement = EFeelSoundPlacement::TwoD;

	/** Socket or bone to attach to. Leave empty for the component origin. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (EditCondition = "Placement == EFeelSoundPlacement::AttachedToTarget", EditConditionHides))
	FName AttachSocketName;

	/** Volume at full intensity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (ClampMin = "0", ClampMax = "4"))
	float VolumeMultiplier = 1.0f;

	/** Scale the volume with the track's intensity, including comfort settings. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	bool bScaleVolumeWithIntensity = true;

	/** Random volume change per play, as a fraction: 0.1 varies the volume by up to 10%. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation", meta = (ClampMin = "0", ClampMax = "1"))
	float VolumeVariation = 0.05f;

	/** Pitch multiplier. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (ClampMin = "0.1", ClampMax = "4"))
	float PitchMultiplier = 1.0f;

	/** Random pitch change per play, as a fraction. A little variation keeps repeated sounds from feeling mechanical. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation", meta = (ClampMin = "0", ClampMax = "0.5"))
	float PitchVariation = 0.05f;

	/** Seconds into the sound to start playing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (ClampMin = "0", Units = "Seconds"))
	float SoundStartTime = 0.0f;

	/** Fade the sound out when the track ends. Only for tracks with a length. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stopping")
	bool bStopAtTrackEnd = false;

	/** Fade the sound out when the recipe is stopped early. Only for tracks with a length. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stopping")
	bool bStopWhenRecipeStops = true;

	/** Fade-out length when the sound is stopped. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stopping", meta = (ClampMin = "0", Units = "Seconds"))
	float FadeOutTime = 0.15f;

	/** Attenuation for positioned sounds. Leave empty to use the sound's own. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (EditCondition = "Placement != EFeelSoundPlacement::TwoD", EditConditionHides))
	TObjectPtr<USoundAttenuation> AttenuationSettings;

	/** Concurrency settings. Leave empty to use the sound's own. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	TObjectPtr<USoundConcurrency> ConcurrencySettings;

	virtual void OnStart_Implementation(const FFeelContext& Context) override;
	virtual void OnStop_Implementation(const FFeelContext& Context, bool bInterrupted) override;
	virtual FGameplayTag GetDefaultChannel_Implementation() const override;
	virtual bool RequiresDuration() const override { return false; }

#if WITH_EDITOR
	virtual void ValidateStep(TArray<FText>& OutErrors, TArray<FText>& OutWarnings) const override;
#endif

private:
	/** World, instance and track of a playing sound. */
	using FSoundKey = TTuple<FObjectKey, int32, int32>;

	static FSoundKey MakeSoundKey(const FFeelContext& Context);

	/** Sounds that may need a fade out, per play. Not saved. */
	TMap<FSoundKey, TWeakObjectPtr<UAudioComponent>> ActiveSounds;
};
