// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "Engine/NetSerialization.h"
#include "FeelParameters.h"
#include "FeelTypes.h"
#include "GameplayTagContainer.h"
#include "FeelReplicationComponent.generated.h"

class UFeelRecipe;

/** Which machines play a networked recipe or event. */
UENUM(BlueprintType)
enum class EFeelNetMode : uint8
{
	/** Every machine plays it. */
	Everyone,
	/** Only the machine that owns this component's actor plays it, for feedback meant for one player. The actor must be owned by a player (its pawn, controller or something they own). */
	OwnerOnly,
	/** Every machine except the owner's plays it, for feedback the owner already played locally. In a single-player game nothing plays. */
	SkipOwner,
};

/** A recipe or event play sent over the network. Only the request travels; every machine evaluates it locally. */
USTRUCT(BlueprintType)
struct FEELCORE_API FFeelNetPlay
{
	GENERATED_BODY()

	/** The recipe to play. Empty when an event is sent instead. */
	UPROPERTY()
	TObjectPtr<UFeelRecipe> Recipe = nullptr;

	/** The event to send when no recipe is set. */
	UPROPERTY()
	FGameplayTag Event;

	/** What it plays on. Widget targets are local UI and are sent as no target. */
	UPROPERTY()
	FFeelTarget Target;

	UPROPERTY()
	float Intensity = 1.0f;

	/** Parameter names and values, as parallel arrays because maps do not replicate. */
	UPROPERTY()
	TArray<FName> ParameterNames;

	UPROPERTY()
	TArray<float> ParameterValues;

	UPROPERTY()
	TObjectPtr<AActor> Instigator = nullptr;

	UPROPERTY()
	FVector_NetQuantizeNormal Direction = FVector::ZeroVector;

	UPROPERTY()
	FVector_NetQuantize10 Location = FVector::ZeroVector;

	UPROPERTY()
	FVector_NetQuantizeNormal Normal = FVector::ZeroVector;

	UPROPERTY()
	FGameplayTagContainer ContextTags;

	UPROPERTY()
	EFeelNetMode Mode = EFeelNetMode::Everyone;

	/** Machines whose local players (character or camera) are all farther than this from the play skip it; a player's own character is always in range. 0 means no limit. */
	UPROPERTY()
	float RelevancyDistance = 0.0f;

	/** Packs a play context. Directions are sent normalized. */
	static FFeelNetPlay Make(UFeelRecipe* InRecipe, const FGameplayTag& InEvent, const FFeelTarget& InTarget, float InIntensity, const FFeelPlayContext& Context, EFeelNetMode InMode, float InRelevancyDistance);

	/** Unpacks the play context. */
	FFeelPlayContext ToContext() const;
};

namespace FeelNet
{
	/** Whether a machine plays a networked request: never on a dedicated server; otherwise decided by the mode and whether this machine owns the actor. */
	FEELCORE_API bool ShouldPlay(EFeelNetMode Mode, ENetMode NetMode, bool bOwnerIsLocal);

	/** Whether a play is close enough to one of this machine's local players (their character or camera), or on something they own. Unknown distances pass. */
	FEELCORE_API bool IsRelevant(const UWorld* World, const FFeelNetPlay& Play);

	/** Whether an actor belongs to a player on this machine: a locally controlled pawn, a local player controller, or owned by one. */
	FEELCORE_API bool IsOwnedLocally(const AActor* Actor);
}

/**
 * Plays recipes and sends Feel Events on several machines in a networked game. Add it to a replicated
 * actor, such as a character or a weapon. Called on the server, the request goes to the machines the mode selects. Called
 * on the client that owns the actor, the client plays at once (no wait for the server) and the server forwards the play to
 * everyone else. Called on any other client, the play stays local. Only the request is sent; each machine evaluates the
 * recipe with its own comfort settings. Plays are cosmetic and unreliable: under heavy packet loss a remote machine may miss one.
 */
UCLASS(ClassGroup = "Feel", meta = (BlueprintSpawnableComponent, DisplayName = "Feel Replication"))
class FEELCORE_API UFeelReplicationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFeelReplicationComponent();

	/**
	 * Plays a recipe on the machines chosen by Mode.
	 * @param Recipe             Recipe to play.
	 * @param Target             What it plays on. Actors and components must exist on every machine (replicated or placed in the level).
	 * @param Intensity          Strength of the play, as in Play Feel.
	 * @param Context            Parameters, instigator, direction, location, normal and tags of the play.
	 * @param Mode               Which machines play it.
	 * @param RelevancyDistance  Machines whose local players are all farther than this from the target (measured from each player's character or camera, whichever is closer) skip the play. A player's own character is always in range. 0 means no limit.
	 * @return Handle of the play on this machine, invalid when this machine does not play it.
	 */
	UFUNCTION(BlueprintCallable, Category = "Feel|Network", meta = (AutoCreateRefTerm = "Context", AdvancedDisplay = "RelevancyDistance"))
	FFeelHandle PlayFeelNetworked(UFeelRecipe* Recipe, const FFeelTarget& Target, float Intensity, const FFeelPlayContext& Context, EFeelNetMode Mode = EFeelNetMode::Everyone, float RelevancyDistance = 0.0f);

	/**
	 * Sends a Feel Event on the machines chosen by Mode. Each machine picks the recipe from its Feel Maps.
	 * @param Event              Event tag.
	 * @param Target             What it plays on. Actors and components must exist on every machine.
	 * @param Intensity          Strength of the play.
	 * @param Context            Parameters, instigator, direction, location, normal and tags of the play.
	 * @param Mode               Which machines play it.
	 * @param RelevancyDistance  Machines whose local players are all farther than this from the target (measured from each player's character or camera, whichever is closer) skip the play. A player's own character is always in range. 0 means no limit.
	 * @return Handle of the play on this machine, invalid when this machine does not play it or no map entry matched.
	 */
	UFUNCTION(BlueprintCallable, Category = "Feel|Network", meta = (AutoCreateRefTerm = "Context", AdvancedDisplay = "RelevancyDistance"))
	FFeelHandle SendFeelEventNetworked(FGameplayTag Event, const FFeelTarget& Target, float Intensity, const FFeelPlayContext& Context, EFeelNetMode Mode = EFeelNetMode::Everyone, float RelevancyDistance = 0.0f);

	/** Plays a received request on this machine if its mode and relevancy allow. Returns the local handle. */
	FFeelHandle HandleNetPlay(const FFeelNetPlay& Play);

private:
	FFeelHandle Dispatch(FFeelNetPlay&& Play);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlay(const FFeelNetPlay& Play);

	UFUNCTION(Client, Unreliable)
	void ClientPlay(const FFeelNetPlay& Play);

	UFUNCTION(Server, Unreliable)
	void ServerPlay(const FFeelNetPlay& Play);

	/** Handle of the last play started on this machine by a received or local request. */
	FFeelHandle LastLocalHandle;
};
