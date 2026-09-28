// Copyright 2026 Billo. All Rights Reserved.

#include "FeelStep_SpawnParticle.h"

#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "FeelTags.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelStep_SpawnParticle)

void UFeelStep_SpawnParticle::OnStart_Implementation(const FFeelContext& Context)
{
	if (!System || !Context.World)
	{
		return;
	}

	const bool bUseNormal = bOrientToNormal && !Context.PlayContext.Normal.IsNearlyZero();
	const FRotator Rotation = bUseNormal ? FRotationMatrix::MakeFromZ(Context.PlayContext.Normal.GetSafeNormal()).Rotator() : FRotator::ZeroRotator;

	UNiagaraComponent* Component = nullptr;
	if (bAttachToTarget && Context.TargetComponent)
	{
		Component = UNiagaraFunctionLibrary::SpawnSystemAttached(System, Context.TargetComponent, AttachSocketName, FVector::ZeroVector, Rotation,
			Scale, EAttachLocation::SnapToTarget, true, ENCPoolMethod::None);
	}
	else
	{
		const FVector SpawnLocation = Location == EFeelSpawnLocation::PlayLocationOrTarget && !Context.PlayContext.Location.IsZero()
			? Context.PlayContext.Location
			: Context.TargetLocation;
		Component = UNiagaraFunctionLibrary::SpawnSystemAtLocation(Context.World, System, SpawnLocation, Rotation, Scale, true, true, ENCPoolMethod::None);
	}

	if (!Component)
	{
		return;
	}

	for (const FFeelParticleParameter& Parameter : Parameters)
	{
		if (Parameter.UserParameter.IsNone())
		{
			continue;
		}
		const float* Value = Parameter.RecipeParameter.IsNone() ? nullptr : Context.PlayContext.Parameters.Find(Parameter.RecipeParameter);
		Component->SetVariableFloat(Parameter.UserParameter, Value ? *Value : Context.Intensity);
	}

	if (bDeactivateWithTrack && Context.TrackDuration > 0.0f)
	{
		ActiveSystems.Add(FParticleKey(FObjectKey(Context.World), Context.InstanceId, Context.TrackIndex), Component);
	}
}

void UFeelStep_SpawnParticle::OnStop_Implementation(const FFeelContext& Context, bool bInterrupted)
{
	TWeakObjectPtr<UNiagaraComponent> WeakComponent;
	if (ActiveSystems.RemoveAndCopyValue(FParticleKey(FObjectKey(Context.World), Context.InstanceId, Context.TrackIndex), WeakComponent))
	{
		if (UNiagaraComponent* Component = WeakComponent.Get())
		{
			Component->Deactivate();
		}
	}
}

FGameplayTag UFeelStep_SpawnParticle::GetDefaultChannel_Implementation() const
{
	return FeelTags::Spawn;
}

#if WITH_EDITOR
void UFeelStep_SpawnParticle::ValidateStep(TArray<FText>& OutErrors, TArray<FText>& OutWarnings) const
{
	if (!System)
	{
		OutErrors.Add(NSLOCTEXT("FeelKit", "SpawnParticleMissingSystem", "Spawn Particle has no particle system."));
	}
}
#endif
