// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FeelStep.h"
#include "UObject/ObjectKey.h"
#include "FeelStep_AudioMix.generated.h"

class UForceFeedbackEffect;
class USoundClass;

/** Base for steps that adjust a sound class while the track plays. */
UCLASS(Abstract)
class FEELCORE_API UFeelStep_SoundClassBase : public UFeelStep
{
	GENERATED_BODY()

public:
	/** Sound class to adjust, including its child classes. Leave empty for the project's default sound class (Project Settings > Audio). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundClass> SoundClass;

	/** Portion of the track spent moving to the full adjustment. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0", ClampMax = "1"))
	float AttackFraction = 0.1f;

	/** Portion of the track spent returning to normal. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0", ClampMax = "1"))
	float ReleaseFraction = 0.4f;

	virtual FGameplayTag GetDefaultChannel_Implementation() const override;
	virtual bool UsesConstantIntensityByDefault() const override { return true; }

protected:
	/** SoundClass, or the project's default sound class. */
	USoundClass* ResolveSoundClass() const;

	/** Adjustment strength at this moment, from 0 to 1. */
	float EvaluateEnvelope(const FFeelStepEvalContext& Context) const;
};

/** Lowers the volume of a sound class and brings it back, such as muffling the world after a loud moment. */
UCLASS(meta = (DisplayName = "Sound Class Duck"))
class FEELCORE_API UFeelStep_SoundClassDuck : public UFeelStep_SoundClassBase
{
	GENERATED_BODY()

public:
	/** Portion of the volume removed at full strength: 1 silences the class. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0", ClampMax = "1"))
	float VolumeReduction = 0.6f;

	virtual void Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const override;
};

/** Bends the pitch of a sound class and brings it back. */
UCLASS(meta = (DisplayName = "Pitch Bend"))
class FEELCORE_API UFeelStep_PitchBend : public UFeelStep_SoundClassBase
{
	GENERATED_BODY()

public:
	/** Pitch change at full strength: -0.3 plays 30% lower, 0.5 plays 50% higher. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "-0.9", ClampMax = "2"))
	float PitchChange = -0.3f;

	virtual void Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const override;
};

/**
 * Filters the high frequencies of a sound class and opens the filter again, for muffled or underwater moments.
 * The cutoff moves between a few preset levels rather than continuously.
 */
UCLASS(meta = (DisplayName = "Low-pass Sweep"))
class FEELCORE_API UFeelStep_LowPassSweep : public UFeelStep_SoundClassBase
{
	GENERATED_BODY()

public:
	/** Cutoff frequency at full strength. Lower is more muffled. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "100", ClampMax = "20000", Units = "Hertz"))
	float CutoffFrequency = 800.0f;

	virtual void Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const override;
};

/**
 * Plays a Force Feedback Effect asset on the controller of the target's player, with its own curves and device
 * properties such as trigger resistance. Comfort can turn it off (Haptics at 0) but cannot scale the asset.
 */
UCLASS(meta = (DisplayName = "Haptic Pattern"))
class FEELCORE_API UFeelStep_HapticPattern : public UFeelStep
{
	GENERATED_BODY()

public:
	/** Force feedback effect to play. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Haptics")
	TObjectPtr<UForceFeedbackEffect> Effect;

	/** Repeat the effect until the track ends. Only for tracks with a length. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Haptics")
	bool bLooping = false;

	/** Stop the effect when the track ends or the recipe stops, even if it is not looping. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Haptics")
	bool bStopWithTrack = true;

	virtual void OnStart_Implementation(const FFeelContext& Context) override;
	virtual void OnStop_Implementation(const FFeelContext& Context, bool bInterrupted) override;
	virtual FGameplayTag GetDefaultChannel_Implementation() const override;
	virtual bool SupportsPreview_Implementation() const override { return false; }
	virtual bool RequiresDuration() const override { return false; }

#if WITH_EDITOR
	virtual void ValidateStep(TArray<FText>& OutErrors, TArray<FText>& OutWarnings) const override;
#endif

private:
	static FName MakeTag(const FFeelContext& Context);
};
