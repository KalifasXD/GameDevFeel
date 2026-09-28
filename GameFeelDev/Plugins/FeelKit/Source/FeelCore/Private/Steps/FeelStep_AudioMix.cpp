// Copyright 2026 Billo. All Rights Reserved.

#include "Steps/FeelStep_AudioMix.h"

#include "FeelAudioDelivery.h"
#include "FeelMotion.h"
#include "FeelOutputSink.h"
#include "FeelTags.h"
#include "GameFramework/ForceFeedbackEffect.h"
#include "GameFramework/ForceFeedbackParameters.h"
#include "GameFramework/PlayerController.h"
#include "Sound/AudioSettings.h"
#include "Sound/SoundClass.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelStep_AudioMix)

FGameplayTag UFeelStep_SoundClassBase::GetDefaultChannel_Implementation() const
{
	return FeelTags::Audio_Mix;
}

USoundClass* UFeelStep_SoundClassBase::ResolveSoundClass() const
{
	if (SoundClass)
	{
		return SoundClass;
	}
	return GetDefault<UAudioSettings>()->GetDefaultSoundClass();
}

float UFeelStep_SoundClassBase::EvaluateEnvelope(const FFeelStepEvalContext& Context) const
{
	return FMath::Clamp(FeelMotion::Envelope(Context.Alpha, AttackFraction, ReleaseFraction) * Context.Intensity, 0.0f, 1.0f);
}

void UFeelStep_SoundClassDuck::Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const
{
	FFeelSoundClassAdjust Adjust;
	Adjust.SoundClass = ResolveSoundClass();
	Adjust.Volume = 1.0f - VolumeReduction * EvaluateEnvelope(Context);
	Sink.AddSoundClassAdjust(Adjust);
}

void UFeelStep_PitchBend::Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const
{
	FFeelSoundClassAdjust Adjust;
	Adjust.SoundClass = ResolveSoundClass();
	Adjust.Pitch = 1.0f + PitchChange * EvaluateEnvelope(Context);
	Sink.AddSoundClassAdjust(Adjust);
}

void UFeelStep_LowPassSweep::Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const
{
	// Sweep on a logarithmic scale, which is how pitch is heard.
	const float Strength = EvaluateEnvelope(Context);
	const float LogCutoff = FMath::Lerp(FMath::Loge(20000.0f), FMath::Loge(FMath::Clamp(CutoffFrequency, 100.0f, 20000.0f)), Strength);

	FFeelSoundClassAdjust Adjust;
	Adjust.SoundClass = ResolveSoundClass();
	Adjust.LowPassFrequency = FMath::Exp(LogCutoff);
	Sink.AddSoundClassAdjust(Adjust);
}

void UFeelStep_HapticPattern::OnStart_Implementation(const FFeelContext& Context)
{
	if (Effect && Context.PlayerController)
	{
		FForceFeedbackParameters Parameters;
		Parameters.Tag = MakeTag(Context);
		Parameters.bLooping = bLooping && Context.TrackDuration > 0.0f;
		Parameters.bIgnoreTimeDilation = true;
		Context.PlayerController->ClientPlayForceFeedback(Effect, Parameters);
	}
}

void UFeelStep_HapticPattern::OnStop_Implementation(const FFeelContext& Context, bool bInterrupted)
{
	const bool bShouldStop = (bLooping || bStopWithTrack) && Context.TrackDuration > 0.0f;
	if (Effect && Context.PlayerController && bShouldStop)
	{
		Context.PlayerController->ClientStopForceFeedback(Effect, MakeTag(Context));
	}
}

FName UFeelStep_HapticPattern::MakeTag(const FFeelContext& Context)
{
	return FName(*FString::Printf(TEXT("FeelKit_%d_%d"), Context.InstanceId, Context.TrackIndex));
}

FGameplayTag UFeelStep_HapticPattern::GetDefaultChannel_Implementation() const
{
	return FeelTags::Haptics;
}

#if WITH_EDITOR
void UFeelStep_HapticPattern::ValidateStep(TArray<FText>& OutErrors, TArray<FText>& OutWarnings) const
{
	if (!Effect)
	{
		OutErrors.Add(NSLOCTEXT("FeelKit", "HapticPatternMissingEffect", "Haptic Pattern has no force feedback effect."));
	}
}
#endif
