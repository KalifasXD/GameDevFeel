// Copyright 2026 Billo. All Rights Reserved.

#include "FeelReplicationComponent.h"

#include "Camera/PlayerCameraManager.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "FeelRecipe.h"
#include "FeelSubsystem.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

FFeelNetPlay FFeelNetPlay::Make(UFeelRecipe* InRecipe, const FGameplayTag& InEvent, const FFeelTarget& InTarget, float InIntensity, const FFeelPlayContext& Context, EFeelNetMode InMode, float InRelevancyDistance)
{
	FFeelNetPlay Play;
	Play.Recipe = InRecipe;
	Play.Event = InEvent;
	Play.Target = InTarget;
	if (Play.Target.Type == EFeelTargetType::Widget)
	{
		Play.Target = FFeelTarget();
	}
	Play.Intensity = InIntensity;
	Play.ParameterNames.Reserve(Context.Parameters.Num());
	Play.ParameterValues.Reserve(Context.Parameters.Num());
	for (const TPair<FName, float>& Pair : Context.Parameters)
	{
		Play.ParameterNames.Add(Pair.Key);
		Play.ParameterValues.Add(Pair.Value);
	}
	Play.Instigator = Context.Instigator;
	Play.Direction = Context.Direction.GetSafeNormal();
	Play.Location = Context.Location;
	Play.Normal = Context.Normal.GetSafeNormal();
	Play.ContextTags = Context.ContextTags;
	Play.Mode = InMode;
	Play.RelevancyDistance = FMath::Max(InRelevancyDistance, 0.0f);
	return Play;
}

FFeelPlayContext FFeelNetPlay::ToContext() const
{
	FFeelPlayContext Context;
	const int32 NumParameters = FMath::Min(ParameterNames.Num(), ParameterValues.Num());
	for (int32 Index = 0; Index < NumParameters; ++Index)
	{
		Context.Parameters.Add(ParameterNames[Index], ParameterValues[Index]);
	}
	Context.Instigator = Instigator;
	Context.Direction = Direction;
	Context.Location = Location;
	Context.Normal = Normal;
	Context.ContextTags = ContextTags;
	return Context;
}

namespace FeelNet
{
	bool ShouldPlay(EFeelNetMode Mode, ENetMode NetMode, bool bOwnerIsLocal)
	{
		if (NetMode == NM_DedicatedServer)
		{
			return false;
		}
		switch (Mode)
		{
		case EFeelNetMode::OwnerOnly:
			return bOwnerIsLocal;
		case EFeelNetMode::SkipOwner:
			return !bOwnerIsLocal;
		case EFeelNetMode::Everyone:
		default:
			return true;
		}
	}

	bool IsRelevant(const UWorld* World, const FFeelNetPlay& Play)
	{
		if (Play.RelevancyDistance <= 0.0f || !World)
		{
			return true;
		}

		FVector PlayLocation;
		if (const USceneComponent* Component = Play.Target.GetSceneComponent())
		{
			PlayLocation = Component->GetComponentLocation();
		}
		else if (const AActor* Actor = Play.Target.GetActor())
		{
			PlayLocation = Actor->GetActorLocation();
		}
		else if (Play.Target.Type == EFeelTargetType::WorldLocation)
		{
			PlayLocation = Play.Target.Location;
		}
		else if (!FVector(Play.Location).IsZero())
		{
			PlayLocation = Play.Location;
		}
		else
		{
			// Camera targets and plays without a location are always at the listener.
			return true;
		}

		// A play on something a local player owns (their own character) is always relevant to that player.
		const AActor* TargetActor = Play.Target.GetActor();
		if (TargetActor && IsOwnedLocally(TargetActor))
		{
			return true;
		}

		// Distance from each local player's character or camera, whichever is closer: a third-person camera sits several
		// meters behind the character, so the camera alone would drop plays right next to the player.
		bool bAnyLocalPlayer = false;
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			const APlayerController* PlayerController = It->Get();
			if (!PlayerController || !PlayerController->IsLocalController())
			{
				continue;
			}
			bAnyLocalPlayer = true;
			double Nearest = TNumericLimits<double>::Max();
			if (PlayerController->PlayerCameraManager)
			{
				Nearest = FVector::Dist(PlayerController->PlayerCameraManager->GetCameraLocation(), PlayLocation);
			}
			if (const APawn* Pawn = PlayerController->GetPawn())
			{
				Nearest = FMath::Min(Nearest, FVector::Dist(Pawn->GetActorLocation(), PlayLocation));
			}
			if (Nearest <= Play.RelevancyDistance)
			{
				return true;
			}
		}
		return !bAnyLocalPlayer;
	}

	bool IsOwnedLocally(const AActor* Actor)
	{
		constexpr int32 MaxOwnerDepth = 8;
		for (int32 Depth = 0; Actor && Depth < MaxOwnerDepth; ++Depth)
		{
			if (const APawn* Pawn = Cast<APawn>(Actor))
			{
				if (Pawn->IsLocallyControlled())
				{
					return true;
				}
			}
			else if (const APlayerController* PlayerController = Cast<APlayerController>(Actor))
			{
				return PlayerController->IsLocalController();
			}
			Actor = Actor->GetOwner();
		}
		return false;
	}
}

UFeelReplicationComponent::UFeelReplicationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

FFeelHandle UFeelReplicationComponent::PlayFeelNetworked(UFeelRecipe* Recipe, const FFeelTarget& Target, float Intensity, const FFeelPlayContext& Context, EFeelNetMode Mode, float RelevancyDistance)
{
	if (!Recipe)
	{
		return FFeelHandle();
	}
	return Dispatch(FFeelNetPlay::Make(Recipe, FGameplayTag(), Target, Intensity, Context, Mode, RelevancyDistance));
}

FFeelHandle UFeelReplicationComponent::SendFeelEventNetworked(FGameplayTag Event, const FFeelTarget& Target, float Intensity, const FFeelPlayContext& Context, EFeelNetMode Mode, float RelevancyDistance)
{
	if (!Event.IsValid())
	{
		return FFeelHandle();
	}
	return Dispatch(FFeelNetPlay::Make(nullptr, Event, Target, Intensity, Context, Mode, RelevancyDistance));
}

FFeelHandle UFeelReplicationComponent::Dispatch(FFeelNetPlay&& Play)
{
	const AActor* Owner = GetOwner();
	const UWorld* World = GetWorld();
	if (!Owner || !World)
	{
		return FFeelHandle();
	}

	LastLocalHandle = FFeelHandle();
	const ENetMode NetMode = World->GetNetMode();

	if (Owner->HasAuthority())
	{
		if (NetMode == NM_Standalone)
		{
			return HandleNetPlay(Play);
		}
		if (Play.Mode == EFeelNetMode::OwnerOnly)
		{
			// The owner may be this machine (listen server host) or a remote client.
			if (FeelNet::IsOwnedLocally(Owner))
			{
				return HandleNetPlay(Play);
			}
			ClientPlay(Play);
			return FFeelHandle();
		}
		MulticastPlay(Play);
		return LastLocalHandle;
	}

	// A client. The owning client plays at once and asks the server to forward the play to everyone else.
	const bool bOwnedHere = FeelNet::IsOwnedLocally(Owner);
	if (bOwnedHere && Play.Mode != EFeelNetMode::OwnerOnly)
	{
		const bool bPlayHere = Play.Mode == EFeelNetMode::Everyone;
		FFeelHandle Handle;
		if (bPlayHere)
		{
			Handle = HandleNetPlay(Play);
		}
		Play.Mode = EFeelNetMode::SkipOwner;
		ServerPlay(Play);
		return Handle;
	}

	// Not owned here (a server call is not possible), or meant only for this owner: a local play.
	return HandleNetPlay(Play);
}

FFeelHandle UFeelReplicationComponent::HandleNetPlay(const FFeelNetPlay& Play)
{
	UWorld* World = GetWorld();
	UFeelSubsystem* Subsystem = World ? World->GetSubsystem<UFeelSubsystem>() : nullptr;
	if (!Subsystem || !FeelNet::ShouldPlay(Play.Mode, World->GetNetMode(), World->GetNetMode() == NM_Standalone || FeelNet::IsOwnedLocally(GetOwner())) || !FeelNet::IsRelevant(World, Play))
	{
		return FFeelHandle();
	}

	const FFeelPlayContext Context = Play.ToContext();
	LastLocalHandle = Play.Recipe
		? Subsystem->PlayFeel(Play.Recipe, Play.Target, Play.Intensity, Context)
		: Subsystem->SendFeelEvent(Play.Event, Play.Target, Play.Intensity, Context);
	return LastLocalHandle;
}

void UFeelReplicationComponent::MulticastPlay_Implementation(const FFeelNetPlay& Play)
{
	HandleNetPlay(Play);
}

void UFeelReplicationComponent::ClientPlay_Implementation(const FFeelNetPlay& Play)
{
	// Sent only to the owner, so the owner check is already satisfied.
	FFeelNetPlay OwnerPlay = Play;
	OwnerPlay.Mode = EFeelNetMode::Everyone;
	HandleNetPlay(OwnerPlay);
}

void UFeelReplicationComponent::ServerPlay_Implementation(const FFeelNetPlay& Play)
{
	MulticastPlay(Play);
}
