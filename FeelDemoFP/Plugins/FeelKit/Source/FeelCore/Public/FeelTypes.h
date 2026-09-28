// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FeelTypes.generated.h"

class AActor;
class APlayerController;
class USceneComponent;
class UWidget;
class UWorld;

/** Identifies one played recipe instance. Safe to keep after the instance ends. */
USTRUCT(BlueprintType)
struct FEELCORE_API FFeelHandle
{
	GENERATED_BODY()

	FFeelHandle() = default;
	explicit FFeelHandle(int32 InId) : Id(InId) {}

	/** True if this handle was returned by a successful play. It may still have finished since. */
	bool IsValid() const { return Id != 0; }
	int32 GetId() const { return Id; }

	bool operator==(const FFeelHandle& Other) const { return Id == Other.Id; }
	bool operator!=(const FFeelHandle& Other) const { return Id != Other.Id; }
	friend uint32 GetTypeHash(const FFeelHandle& Handle) { return ::GetTypeHash(Handle.Id); }

private:
	UPROPERTY()
	int32 Id = 0;
};

/** Kinds of things a recipe can play on. */
UENUM(BlueprintType)
enum class EFeelTargetType : uint8
{
	/** No target: camera and screen effects go to the first local player. */
	None,
	/** An actor; actor effects use its root component, or its visible child components when the root takes part in collision (such as a character's capsule). */
	Actor,
	/** A specific scene component. */
	SceneComponent,
	/** A point in the world. */
	WorldLocation,
	/** The camera of a local player. */
	LocalPlayerCamera,
	/** A UMG widget. Widget steps apply to it; camera and screen effects go to its owning player. */
	Widget,
};

/** What a recipe plays on. */
USTRUCT(BlueprintType)
struct FEELCORE_API FFeelTarget
{
	GENERATED_BODY()

	/** Kind of target. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel")
	EFeelTargetType Type = EFeelTargetType::None;

	/** Target actor when Type is Actor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel", meta = (EditCondition = "Type == EFeelTargetType::Actor", EditConditionHides))
	TObjectPtr<AActor> Actor = nullptr;

	/** Target component when Type is SceneComponent. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel", meta = (EditCondition = "Type == EFeelTargetType::SceneComponent", EditConditionHides))
	TObjectPtr<USceneComponent> Component = nullptr;

	/** World location when Type is WorldLocation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel", meta = (EditCondition = "Type == EFeelTargetType::WorldLocation", EditConditionHides))
	FVector Location = FVector::ZeroVector;

	/** Local player index when Type is LocalPlayerCamera. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel", meta = (ClampMin = "0", EditCondition = "Type == EFeelTargetType::LocalPlayerCamera", EditConditionHides))
	int32 LocalPlayerIndex = 0;

	/** Target widget when Type is Widget. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel", meta = (EditCondition = "Type == EFeelTargetType::Widget", EditConditionHides))
	TObjectPtr<UWidget> Widget = nullptr;

	static FFeelTarget FromActor(AActor* InActor);
	static FFeelTarget FromComponent(USceneComponent* InComponent);
	static FFeelTarget AtLocation(const FVector& InLocation);
	static FFeelTarget FromLocalPlayerCamera(int32 InLocalPlayerIndex);
	static FFeelTarget FromWidget(UWidget* InWidget);

	/** True for targets that point at an object which can be destroyed during playback. */
	bool UsesObject() const { return Type == EFeelTargetType::Actor || Type == EFeelTargetType::SceneComponent || Type == EFeelTargetType::Widget; }

	/** Actor of the target, if any. */
	AActor* GetActor() const;

	/** Scene component that actor effects (such as Scale Punch) apply to, if any. */
	USceneComponent* GetSceneComponent() const;

	/**
	 * Local player controller whose camera receives camera, screen and controller effects. Targets that belong to no player
	 * (props, AI) use the first local player. Targets that belong to a player on another machine in a networked game return
	 * none: that player's own machine shows those effects.
	 */
	APlayerController* ResolvePlayerController(const UWorld* World) const;

	/**
	 * Whether the target belongs to a player on another machine: a pawn with a player state that is not locally controlled,
	 * a player controller that is not local, or something owned by one of those.
	 */
	bool BelongsToRemotePlayer() const;
};
