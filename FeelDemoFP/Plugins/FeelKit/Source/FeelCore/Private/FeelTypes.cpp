// Copyright 2026 Billo. All Rights Reserved.

#include "FeelTypes.h"

#include "Components/SceneComponent.h"
#include "Components/Widget.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelTypes)

FFeelTarget FFeelTarget::FromActor(AActor* InActor)
{
	FFeelTarget Target;
	Target.Type = EFeelTargetType::Actor;
	Target.Actor = InActor;
	return Target;
}

FFeelTarget FFeelTarget::FromComponent(USceneComponent* InComponent)
{
	FFeelTarget Target;
	Target.Type = EFeelTargetType::SceneComponent;
	Target.Component = InComponent;
	return Target;
}

FFeelTarget FFeelTarget::AtLocation(const FVector& InLocation)
{
	FFeelTarget Target;
	Target.Type = EFeelTargetType::WorldLocation;
	Target.Location = InLocation;
	return Target;
}

FFeelTarget FFeelTarget::FromLocalPlayerCamera(int32 InLocalPlayerIndex)
{
	FFeelTarget Target;
	Target.Type = EFeelTargetType::LocalPlayerCamera;
	Target.LocalPlayerIndex = InLocalPlayerIndex;
	return Target;
}

FFeelTarget FFeelTarget::FromWidget(UWidget* InWidget)
{
	FFeelTarget Target;
	Target.Type = EFeelTargetType::Widget;
	Target.Widget = InWidget;
	return Target;
}

AActor* FFeelTarget::GetActor() const
{
	switch (Type)
	{
	case EFeelTargetType::Actor:
		return Actor;
	case EFeelTargetType::SceneComponent:
		return Component ? Component->GetOwner() : nullptr;
	default:
		return nullptr;
	}
}

USceneComponent* FFeelTarget::GetSceneComponent() const
{
	switch (Type)
	{
	case EFeelTargetType::Actor:
		return Actor ? Actor->GetRootComponent() : nullptr;
	case EFeelTargetType::SceneComponent:
		return Component;
	default:
		return nullptr;
	}
}

APlayerController* FFeelTarget::ResolvePlayerController(const UWorld* World) const
{
	if (!World)
	{
		return nullptr;
	}

	if (Type == EFeelTargetType::LocalPlayerCamera)
	{
		return UGameplayStatics::GetPlayerController(World, LocalPlayerIndex);
	}

	if (Type == EFeelTargetType::Widget && Widget)
	{
		if (APlayerController* Owner = Widget->GetOwningPlayer())
		{
			return Owner;
		}
	}

	// The target's own local player: a locally controlled pawn, a local controller, or something they own.
	constexpr int32 MaxOwnerDepth = 8;
	AActor* Owner = GetActor();
	for (int32 Depth = 0; Owner && Depth < MaxOwnerDepth; ++Depth)
	{
		if (const APawn* Pawn = Cast<APawn>(Owner))
		{
			APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController());
			if (PlayerController && PlayerController->IsLocalController())
			{
				return PlayerController;
			}
		}
		else if (APlayerController* PlayerController = Cast<APlayerController>(Owner))
		{
			if (PlayerController->IsLocalController())
			{
				return PlayerController;
			}
		}
		Owner = Owner->GetOwner();
	}

	return BelongsToRemotePlayer() ? nullptr : World->GetFirstPlayerController();
}

bool FFeelTarget::BelongsToRemotePlayer() const
{
	if (Type != EFeelTargetType::Actor && Type != EFeelTargetType::SceneComponent)
	{
		return false;
	}

	constexpr int32 MaxOwnerDepth = 8;
	const AActor* Owner = GetActor();
	if (!Owner || Owner->GetNetMode() == NM_Standalone)
	{
		// Single player: every player is on this machine.
		return false;
	}
	for (int32 Depth = 0; Owner && Depth < MaxOwnerDepth; ++Depth)
	{
		if (const APawn* Pawn = Cast<APawn>(Owner))
		{
			if (Pawn->IsLocallyControlled())
			{
				return false;
			}
			// Remote human players: a controller that is not local (server side), or a human player state without a
			// controller (client side, where other players' controllers do not exist). Bots are not players.
			if (const APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController()))
			{
				return !PlayerController->IsLocalController();
			}
			const APlayerState* PlayerState = Pawn->GetPlayerState();
			if (PlayerState && !PlayerState->IsABot() && !Pawn->GetController())
			{
				return true;
			}
		}
		else if (const APlayerController* PlayerController = Cast<APlayerController>(Owner))
		{
			return !PlayerController->IsLocalController();
		}
		Owner = Owner->GetOwner();
	}
	return false;
}
