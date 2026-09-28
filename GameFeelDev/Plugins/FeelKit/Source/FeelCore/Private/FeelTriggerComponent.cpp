// Copyright 2026 Billo. All Rights Reserved.

#include "FeelTriggerComponent.h"

#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "FeelMap.h"
#include "FeelRecipe.h"
#include "FeelSubsystem.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelTriggerComponent)

UFeelTriggerComponent::UFeelTriggerComponent()
{
	// Ticks only when an entry needs it (jumps and launches have no engine event to bind).
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UFeelTriggerComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	if (HasEntryFor(EFeelTriggerEvent::TakeAnyDamage))
	{
		Owner->OnTakeAnyDamage.AddDynamic(this, &UFeelTriggerComponent::HandleTakeAnyDamage);
	}
	if (HasEntryFor(EFeelTriggerEvent::Landed))
	{
		if (ACharacter* Character = Cast<ACharacter>(Owner))
		{
			Character->LandedDelegate.AddDynamic(this, &UFeelTriggerComponent::HandleLanded);
		}
	}
	if (HasEntryFor(EFeelTriggerEvent::ComponentHit))
	{
		if (UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(Owner->GetRootComponent()))
		{
			Root->OnComponentHit.AddDynamic(this, &UFeelTriggerComponent::HandleComponentHit);
		}
	}
	if (HasEntryFor(EFeelTriggerEvent::BeginOverlap))
	{
		Owner->OnActorBeginOverlap.AddDynamic(this, &UFeelTriggerComponent::HandleBeginOverlap);
	}
	if (HasEntryFor(EFeelTriggerEvent::EndOverlap))
	{
		Owner->OnActorEndOverlap.AddDynamic(this, &UFeelTriggerComponent::HandleEndOverlap);
	}
	if (HasEntryFor(EFeelTriggerEvent::Jumped) || HasEntryFor(EFeelTriggerEvent::AirJumped) || HasEntryFor(EFeelTriggerEvent::Launched))
	{
		if (ACharacter* Character = Cast<ACharacter>(Owner))
		{
			LastJumpCount = Character->JumpCurrentCount;
			if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
			{
				Movement->PrimaryComponentTick.AddPrerequisite(this, PrimaryComponentTick);
			}
			UpdateTickOrder();
			SetComponentTickEnabled(true);
		}
	}
}

void UFeelTriggerComponent::UpdateTickOrder()
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	AController* Controller = Pawn ? Pawn->GetController() : nullptr;
	if (Controller == OrderedController.Get())
	{
		return;
	}
	if (AController* Previous = OrderedController.Get())
	{
		RemoveTickPrerequisiteActor(Previous);
	}
	if (Controller)
	{
		AddTickPrerequisiteActor(Controller);
	}
	OrderedController = Controller;
}

void UFeelTriggerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;
	if (!Movement)
	{
		return;
	}
	UpdateTickOrder();

	// Launches wait in the movement component until its next update, which runs right after this tick.
	if (!Movement->PendingLaunchVelocity.IsNearlyZero())
	{
		const FVector Launch = Movement->PendingLaunchVelocity;
		FireEventInternal(EFeelTriggerEvent::Launched, static_cast<float>(Launch.Size()), nullptr, Launch.GetSafeNormal());
	}

	// A jump raises the character's jump count during the movement update; a landing resets it.
	const int32 JumpCount = Character->JumpCurrentCount;
	if (JumpCount > LastJumpCount)
	{
		const bool bFromGround = LastJumpCount == 0;
		FireEventInternal(bFromGround ? EFeelTriggerEvent::Jumped : EFeelTriggerEvent::AirJumped, static_cast<float>(bFromGround ? 1 : JumpCount), nullptr, FVector::UpVector);
	}
	LastJumpCount = JumpCount;
}

void UFeelTriggerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AActor* Owner = GetOwner())
	{
		Owner->OnTakeAnyDamage.RemoveDynamic(this, &UFeelTriggerComponent::HandleTakeAnyDamage);
		Owner->OnActorBeginOverlap.RemoveDynamic(this, &UFeelTriggerComponent::HandleBeginOverlap);
		Owner->OnActorEndOverlap.RemoveDynamic(this, &UFeelTriggerComponent::HandleEndOverlap);
		if (ACharacter* Character = Cast<ACharacter>(Owner))
		{
			Character->LandedDelegate.RemoveDynamic(this, &UFeelTriggerComponent::HandleLanded);
			if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
			{
				Movement->PrimaryComponentTick.RemovePrerequisite(this, PrimaryComponentTick);
			}
		}
		if (UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(Owner->GetRootComponent()))
		{
			Root->OnComponentHit.RemoveDynamic(this, &UFeelTriggerComponent::HandleComponentHit);
		}
	}
	if (AController* Controller = OrderedController.Get())
	{
		RemoveTickPrerequisiteActor(Controller);
	}
	OrderedController.Reset();
	Super::EndPlay(EndPlayReason);
}

bool UFeelTriggerComponent::HasEntryFor(EFeelTriggerEvent Event) const
{
	return Triggers.ContainsByPredicate([Event](const FFeelTriggerEntry& Entry) { return Entry.Event == Event; });
}

bool UFeelTriggerComponent::IsPlayerActor(const AActor* Actor)
{
	if (const APawn* Pawn = Cast<APawn>(Actor))
	{
		return Pawn->IsPlayerControlled();
	}
	return Cast<APlayerController>(Actor) != nullptr;
}

void UFeelTriggerComponent::FireEvent(EFeelTriggerEvent Event, float EventValue, AActor* OtherActor)
{
	FireEventInternal(Event, EventValue, OtherActor, FVector::ZeroVector);
}

void UFeelTriggerComponent::FireEventInternal(EFeelTriggerEvent Event, float EventValue, AActor* OtherActor, const FVector& Direction)
{
	AActor* Owner = GetOwner();
	UFeelSubsystem* Subsystem = UFeelSubsystem::Get(this);
	if (!Owner || !Subsystem)
	{
		return;
	}

	for (const FFeelTriggerEntry& Entry : Triggers)
	{
		if (Entry.Event != Event)
		{
			continue;
		}
		if (Entry.bOnlyForPlayers && !IsPlayerActor(OtherActor))
		{
			continue;
		}

		const bool bOnOther = Entry.PlayOn == EFeelTriggerPlayOn::OtherActor && OtherActor && OtherActor != Owner;
		AActor* PlayActor = bOnOther ? OtherActor : Owner;
		FFeelTarget Target = FFeelTarget::FromActor(PlayActor);
		if (Entry.bTargetSkeletalMesh)
		{
			if (USkeletalMeshComponent* Mesh = PlayActor->FindComponentByClass<USkeletalMeshComponent>())
			{
				Target = FFeelTarget::FromComponent(Mesh);
			}
		}

		FFeelPlayContext Context;
		Context.ContextTags = Entry.ContextTags;
		Context.Direction = Direction;
		if (bOnOther)
		{
			// where it happened: the owner (for example the jump pad the player stepped on)
			Context.Location = Owner->GetActorLocation();
		}
		if (Entry.bOtherActorAsInstigator && OtherActor && OtherActor != Owner)
		{
			Context.Instigator = bOnOther ? Owner : OtherActor;
		}
		if (!Entry.ValueParameter.IsNone())
		{
			Context.Parameters.Add(Entry.ValueParameter, EventValue);
		}

		if (Entry.Recipe)
		{
			Subsystem->PlayFeel(Entry.Recipe, Target, Entry.Intensity, Context);
		}
		else if (Entry.FeelEvent.IsValid())
		{
			Subsystem->SendFeelEvent(Entry.FeelEvent, Target, Entry.Intensity, Context);
		}
	}
}

void UFeelTriggerComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	FireEvent(EFeelTriggerEvent::TakeAnyDamage, Damage, DamageCauser);
}

void UFeelTriggerComponent::HandleLanded(const FHitResult& Hit)
{
	float LandingSpeed = 0.0f;
	if (const ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		if (const UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
		{
			LandingSpeed = FMath::Max(static_cast<float>(-Movement->Velocity.Z), 0.0f);
		}
	}
	FireEvent(EFeelTriggerEvent::Landed, LandingSpeed, Hit.GetActor());
}

void UFeelTriggerComponent::HandleComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	FireEvent(EFeelTriggerEvent::ComponentHit, static_cast<float>(NormalImpulse.Size()), OtherActor);
}

void UFeelTriggerComponent::HandleBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	FireEvent(EFeelTriggerEvent::BeginOverlap, 0.0f, OtherActor);
}

void UFeelTriggerComponent::HandleEndOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	FireEvent(EFeelTriggerEvent::EndOverlap, 0.0f, OtherActor);
}
