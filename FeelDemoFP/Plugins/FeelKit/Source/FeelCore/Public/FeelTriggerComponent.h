// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FeelParameters.h"
#include "GameplayTagContainer.h"
#include "FeelTriggerComponent.generated.h"

class AController;
class UDamageType;
class UFeelMap;
class UFeelRecipe;
class UPrimitiveComponent;
struct FHitResult;

/** Actor events a Feel Trigger component can react to (TRG-002). */
UENUM(BlueprintType)
enum class EFeelTriggerEvent : uint8
{
	/** The owner receives damage (any damage type). Event value: the damage amount. */
	TakeAnyDamage UMETA(DisplayName = "Take Any Damage"),
	/** The owner, a Character, lands after falling. Event value: the downward speed at landing, in cm/s. */
	Landed,
	/** The owner's root component reports a blocking hit. Event value: the length of the normal impulse. */
	ComponentHit UMETA(DisplayName = "Component Hit"),
	/** Another actor starts overlapping the owner. Event value: 0. */
	BeginOverlap UMETA(DisplayName = "Begin Overlap"),
	/** Another actor stops overlapping the owner. Event value: 0. */
	EndOverlap UMETA(DisplayName = "End Overlap"),
	/** The owner, a Character, jumps from the ground (including a late jump just after walking off a ledge). Event value: the jump number, 1. */
	Jumped,
	/** The owner, a Character, jumps again while already in the air (double or triple jump). Event value: the jump number, 2 or more. */
	AirJumped UMETA(DisplayName = "Air Jumped"),
	/**
	 * The owner, a Character, is launched: thrown by gameplay rather than by its own movement, such as a wall jump, a jump
	 * pad or a knockback (Launch Character). Event value: the launch speed in cm/s. The play's Direction is the launch direction.
	 */
	Launched,
};

/** What a Feel Trigger entry plays its recipe on. */
UENUM(BlueprintType)
enum class EFeelTriggerPlayOn : uint8
{
	/** The actor that owns the Feel Trigger component. */
	Owner,
	/**
	 * The other actor of the event (the damage causer, the hit actor, the actor that started or stopped overlapping).
	 * Use it for things in the level that act on whoever touches them, such as a jump pad. Falls back to the owner when
	 * the event has no other actor.
	 */
	OtherActor UMETA(DisplayName = "Other Actor"),
};

/** What a Feel Trigger component plays when one event happens. */
USTRUCT(BlueprintType)
struct FEELCORE_API FFeelTriggerEntry
{
	GENERATED_BODY()

	/** Actor event that fires this entry. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
	EFeelTriggerEvent Event = EFeelTriggerEvent::TakeAnyDamage;

	/** Recipe to play. When empty, Feel Event is sent instead. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
	TObjectPtr<UFeelRecipe> Recipe = nullptr;

	/** Event sent through the Feel Maps when no recipe is set, so maps choose the recipe. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger", meta = (EditCondition = "Recipe == nullptr"))
	FGameplayTag FeelEvent;

	/**
	 * Recipe parameter that receives the event value (damage amount, landing speed or hit impulse). Map that parameter on
	 * tracks to scale them by it (TRG-004). Leave empty to ignore the value.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
	FName ValueParameter;

	/** Intensity of the play. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger", meta = (ClampMin = "0"))
	float Intensity = 1.0f;

	/**
	 * Play on the owner's first skeletal mesh component instead of the owner actor, so actor effects such as scale apply
	 * to the visible mesh of characters rather than their collision capsule.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
	bool bTargetSkeletalMesh = true;

	/**
	 * Pass the other actor of the event (damage causer, hit or overlapping actor) as the play's instigator. When the
	 * entry plays on the other actor, the owner becomes the instigator instead.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
	bool bOtherActorAsInstigator = true;

	/** Where the recipe plays: on the owner, or on the other actor of the event. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
	EFeelTriggerPlayOn PlayOn = EFeelTriggerPlayOn::Owner;

	/**
	 * Fire only when the other actor of the event is a pawn controlled by a player (or that player's controller), so
	 * enemies, props or projectiles do not set it off.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
	bool bOnlyForPlayers = false;

	/** Context tags added to every play of this entry. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
	FGameplayTagContainer ContextTags;
};

/**
 * Plays recipes or sends Feel Events when common events happen to its owner, with no Blueprint wiring (TRG-001).
 * Also holds Feel Maps that Send Feel Event checks first when this actor is the target.
 */
UCLASS(ClassGroup = (Feel), meta = (BlueprintSpawnableComponent, DisplayName = "Feel Trigger"))
class FEELCORE_API UFeelTriggerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFeelTriggerComponent();

	/** What to play for each event. Several entries may use the same event. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel")
	TArray<FFeelTriggerEntry> Triggers;

	/** Feel Maps checked before the project's maps when a Feel Event targets this actor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel")
	TArray<TObjectPtr<UFeelMap>> FeelMaps;

	/**
	 * Fires every entry for an event, as if it happened. Useful for events the component does not bind itself.
	 * @param Event			Which entries to fire: those set to this event.
	 * @param EventValue	The event's value, passed to each entry's Value Parameter (for example a fall speed or damage).
	 * @param OtherActor	The other actor of the event, used by Other Actor as Instigator and by Play On set to Other Actor. Can be empty.
	 */
	UFUNCTION(BlueprintCallable, Category = "Feel")
	void FireEvent(EFeelTriggerEvent Event, float EventValue, AActor* OtherActor);

	/** True for a pawn controlled by a player, or a player controller. */
	static bool IsPlayerActor(const AActor* Actor);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void FireEventInternal(EFeelTriggerEvent Event, float EventValue, AActor* OtherActor, const FVector& Direction);

	/** Ticks between the owner's controller (where input launches and jumps are requested) and its character movement (where they happen). */
	void UpdateTickOrder();

	/** Jump count seen on the previous tick, to notice new jumps. */
	int32 LastJumpCount = 0;

	/** Controller the tick order was set up against. */
	TWeakObjectPtr<AController> OrderedController;

	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION()
	void HandleLanded(const FHitResult& Hit);

	UFUNCTION()
	void HandleComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void HandleBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

	UFUNCTION()
	void HandleEndOverlap(AActor* OverlappedActor, AActor* OtherActor);

	bool HasEntryFor(EFeelTriggerEvent Event) const;
};
