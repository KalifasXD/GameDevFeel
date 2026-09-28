// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FeelStep.h"
#include "Steps/FeelStep_Spawn.h"
#include "UObject/ObjectKey.h"
#include "UObject/WeakObjectPtr.h"
#include "FeelStep_SpawnParticle.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;

/** Passes a value into a Niagara user parameter when the system spawns. */
USTRUCT(BlueprintType)
struct FEELNIAGARA_API FFeelParticleParameter
{
	GENERATED_BODY()

	/** Float user parameter of the system, without the "User." prefix. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	FName UserParameter;

	/** Recipe parameter whose value is passed. Leave empty to pass the track's intensity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	FName RecipeParameter;
};

/**
 * Spawns a Niagara particle system at the target or the play location, optionally attached and oriented by the play
 * context's Normal. Plays in the editor preview. Part of the optional FeelNiagara module.
 */
UCLASS(meta = (DisplayName = "Spawn Particle"))
class FEELNIAGARA_API UFeelStep_SpawnParticle : public UFeelStep
{
	GENERATED_BODY()

public:
	/** Particle system to spawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle")
	TObjectPtr<UNiagaraSystem> System;

	/** Where the system appears when not attached. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle")
	EFeelSpawnLocation Location = EFeelSpawnLocation::PlayLocationOrTarget;

	/** Follow the target component instead of staying where it spawned. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle")
	bool bAttachToTarget = false;

	/** Socket or bone to attach to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle", meta = (EditCondition = "bAttachToTarget"))
	FName AttachSocketName;

	/** Point the system's up axis along the play context's Normal when the play passed one. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle")
	bool bOrientToNormal = true;

	/** Scale of the spawned system. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle")
	FVector Scale = FVector::OneVector;

	/** Values passed into user parameters of the system. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle")
	TArray<FFeelParticleParameter> Parameters;

	/** Stop the system spawning new particles when the track ends or the recipe stops. Only for tracks with a length. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle")
	bool bDeactivateWithTrack = true;

	virtual void OnStart_Implementation(const FFeelContext& Context) override;
	virtual void OnStop_Implementation(const FFeelContext& Context, bool bInterrupted) override;
	virtual FGameplayTag GetDefaultChannel_Implementation() const override;
	virtual bool RequiresDuration() const override { return false; }

#if WITH_EDITOR
	virtual void ValidateStep(TArray<FText>& OutErrors, TArray<FText>& OutWarnings) const override;
#endif

private:
	using FParticleKey = TTuple<FObjectKey, int32, int32>;

	/** Systems that may need deactivating, per play. Not saved. */
	TMap<FParticleKey, TWeakObjectPtr<UNiagaraComponent>> ActiveSystems;
};
