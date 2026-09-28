// Copyright 2026 Billo. All Rights Reserved.

#include "FeelGameplayCue.h"

#include "Engine/World.h"
#include "FeelParameters.h"
#include "FeelSubsystem.h"
#include "GameplayEffectTypes.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_MODULE(FDefaultModuleImpl, FeelGAS);

FFeelPlayContext FFeelGameplayCueSettings::MakeContext(const FGameplayCueParameters& Parameters) const
{
	FFeelPlayContext Context;
	if (!RawMagnitudeParameter.IsNone())
	{
		Context.Parameters.Add(RawMagnitudeParameter, Parameters.RawMagnitude);
	}
	if (!NormalizedMagnitudeParameter.IsNone())
	{
		Context.Parameters.Add(NormalizedMagnitudeParameter, Parameters.NormalizedMagnitude);
	}

	AActor* Instigator = bEffectCauserAsInstigator ? Parameters.EffectCauser.Get() : Parameters.Instigator.Get();
	Context.Instigator = Instigator ? Instigator : Parameters.Instigator.Get();
	Context.Location = Parameters.Location;
	Context.Normal = Parameters.Normal;
	Context.ContextTags.AppendTags(Parameters.AggregatedSourceTags);
	Context.ContextTags.AppendTags(Parameters.AggregatedTargetTags);
	return Context;
}

FFeelHandle FFeelGameplayCueSettings::Play(AActor* CueTarget, const FGameplayCueParameters& Parameters) const
{
	UWorld* World = CueTarget ? CueTarget->GetWorld() : nullptr;
	UFeelSubsystem* Subsystem = World ? World->GetSubsystem<UFeelSubsystem>() : nullptr;
	if (!Subsystem)
	{
		return FFeelHandle();
	}

	const float PlayIntensity = bScaleIntensityByMagnitude ? Intensity * FMath::Clamp(Parameters.NormalizedMagnitude, 0.0f, 1.0f) : Intensity;
	const FFeelPlayContext Context = MakeContext(Parameters);
	const FFeelTarget Target = FFeelTarget::FromActor(CueTarget);
	if (Recipe)
	{
		return Subsystem->PlayFeel(Recipe, Target, PlayIntensity, Context);
	}

	const FGameplayTag EventTag = Event.IsValid() ? Event : Parameters.OriginalTag;
	return EventTag.IsValid() ? Subsystem->SendFeelEvent(EventTag, Target, PlayIntensity, Context) : FFeelHandle();
}

void UFeelGameplayCueNotify::HandleGameplayCue(AActor* MyTarget, EGameplayCueEvent::Type EventType, const FGameplayCueParameters& Parameters)
{
	Super::HandleGameplayCue(MyTarget, EventType, Parameters);

	if (EventType == EGameplayCueEvent::Executed || EventType == EGameplayCueEvent::OnActive)
	{
		FGameplayCueParameters CueParameters = Parameters;
		if (!CueParameters.OriginalTag.IsValid())
		{
			CueParameters.OriginalTag = GameplayCueTag;
		}
		Feel.Play(MyTarget, CueParameters);
	}
}

void AFeelGameplayCueNotifyActor::HandleGameplayCue(AActor* MyTarget, EGameplayCueEvent::Type EventType, const FGameplayCueParameters& Parameters)
{
	Super::HandleGameplayCue(MyTarget, EventType, Parameters);

	UWorld* World = GetWorld();
	UFeelSubsystem* Subsystem = World ? World->GetSubsystem<UFeelSubsystem>() : nullptr;
	if (!Subsystem)
	{
		return;
	}

	switch (EventType)
	{
	case EGameplayCueEvent::OnActive:
	case EGameplayCueEvent::WhileActive:
		// WhileActive also arrives when a cue becomes relevant late (join in progress); play once either way.
		if (!Subsystem->IsPlaying(ActiveHandle))
		{
			FGameplayCueParameters CueParameters = Parameters;
			if (!CueParameters.OriginalTag.IsValid())
			{
				CueParameters.OriginalTag = GameplayCueTag;
			}
			ActiveHandle = Feel.Play(MyTarget, CueParameters);
		}
		break;

	case EGameplayCueEvent::Removed:
		if (ActiveHandle.IsValid())
		{
			if (bStopOnRemove)
			{
				Subsystem->StopFeel(ActiveHandle, true);
			}
			else
			{
				Subsystem->ReleaseFeel(ActiveHandle);
			}
			ActiveHandle = FFeelHandle();
		}
		break;

	default:
		break;
	}
}

bool AFeelGameplayCueNotifyActor::Recycle()
{
	ActiveHandle = FFeelHandle();
	return Super::Recycle();
}
