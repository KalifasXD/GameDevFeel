// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FeelTypes.h"
#include "GameplayCueNotify_Actor.h"
#include "GameplayCueNotify_Static.h"
#include "GameplayTagContainer.h"
#include "FeelGameplayCue.generated.h"

class UFeelRecipe;
struct FFeelPlayContext;
struct FGameplayCueParameters;

/** What a FeelKit gameplay cue plays and how cue information becomes play information. */
USTRUCT(BlueprintType)
struct FEELGAS_API FFeelGameplayCueSettings
{
	GENERATED_BODY()

	/** Recipe to play. When empty, Event is sent instead. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel")
	TObjectPtr<UFeelRecipe> Recipe = nullptr;

	/** Feel Event to send when no recipe is set, so Feel Maps choose the recipe. Empty uses the cue's own tag. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel")
	FGameplayTag Event;

	/** Multiplies the play's intensity by the cue's normalized magnitude (0 to 1). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel")
	bool bScaleIntensityByMagnitude = false;

	/** Intensity of the play, before the magnitude scale. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel", meta = (ClampMin = "0"))
	float Intensity = 1.0f;

	/** Recipe parameter that receives the cue's raw magnitude. Empty sends none. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel")
	FName RawMagnitudeParameter;

	/** Recipe parameter that receives the cue's normalized magnitude. Empty sends none. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel")
	FName NormalizedMagnitudeParameter;

	/** Use the effect causer (such as a weapon or projectile) as the play's instigator instead of the cue's instigator. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel")
	bool bEffectCauserAsInstigator = false;

	/** Builds the play context from cue parameters: magnitudes, instigator, location, normal and the effect's source and target tags. */
	FFeelPlayContext MakeContext(const FGameplayCueParameters& Parameters) const;

	/** Plays the recipe or sends the event on the cue target. */
	FFeelHandle Play(AActor* CueTarget, const FGameplayCueParameters& Parameters) const;
};

/**
 * Gameplay Cue notify that plays a FeelKit recipe or sends a Feel Event when the cue is executed or added. For one-shot
 * feedback such as hits and pickups. For feedback that lasts while a gameplay effect is active, use Feel Gameplay Cue
 * Notify (Actor). Set the Gameplay Cue Tag as for any static cue notify.
 */
UCLASS(Blueprintable, meta = (DisplayName = "Feel Gameplay Cue Notify"))
class FEELGAS_API UFeelGameplayCueNotify : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Feel", meta = (ShowOnlyInnerProperties))
	FFeelGameplayCueSettings Feel;

	virtual void HandleGameplayCue(AActor* MyTarget, EGameplayCueEvent::Type EventType, const FGameplayCueParameters& Parameters) override;
};

/**
 * Gameplay Cue notify actor that plays a FeelKit recipe while its gameplay cue is active: it starts when the cue is added
 * and ends when the cue is removed (see Stop On Remove). Use a recipe with a sustain region for feedback that loops for as
 * long as the effect lasts.
 */
UCLASS(Blueprintable, meta = (DisplayName = "Feel Gameplay Cue Notify (Actor)"))
class FEELGAS_API AFeelGameplayCueNotifyActor : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Feel", meta = (ShowOnlyInnerProperties))
	FFeelGameplayCueSettings Feel;

	/** When the cue is removed: on, the play stops with a blend out; off, a sustained recipe is released and plays its ending, and any other recipe plays to its end. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Feel")
	bool bStopOnRemove = false;

	virtual void HandleGameplayCue(AActor* MyTarget, EGameplayCueEvent::Type EventType, const FGameplayCueParameters& Parameters) override;
	virtual bool Recycle() override;

private:
	FFeelHandle ActiveHandle;
};
