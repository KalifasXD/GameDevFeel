// Copyright 2026 Billo. All Rights Reserved.

#include "FeelInputComponent.h"

#include "EnhancedInputComponent.h"
#include "Engine/World.h"
#include "FeelParameters.h"
#include "FeelPlaybackClock.h"
#include "FeelRecipe.h"
#include "FeelSubsystem.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "Modules/ModuleManager.h"
#include "TimerManager.h"

IMPLEMENT_MODULE(FDefaultModuleImpl, FeelEnhancedInput);

UFeelInputComponent::UFeelInputComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFeelInputComponent::BeginPlay()
{
	Super::BeginPlay();

	RefreshBinding();

	// A pawn's input component is created when a local controller possesses it, which can be after BeginPlay, and it is
	// recreated on each possession. A light check keeps the bindings attached to the current one.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(RefreshTimer, FTimerDelegate::CreateUObject(this, &UFeelInputComponent::RefreshBinding), 0.25f, true);
	}
}

void UFeelInputComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RefreshTimer);
	}
	Unbind();
	Super::EndPlay(EndPlayReason);
}

UEnhancedInputComponent* UFeelInputComponent::FindOwnerInputComponent() const
{
	const AActor* Owner = GetOwner();
	return Owner ? Cast<UEnhancedInputComponent>(Owner->InputComponent) : nullptr;
}

void UFeelInputComponent::RefreshBinding()
{
	UEnhancedInputComponent* Current = FindOwnerInputComponent();
	if (Current != BoundComponent.Get())
	{
		BindTo(Current);
	}
}

void UFeelInputComponent::Unbind()
{
	if (UEnhancedInputComponent* InputComponent = BoundComponent.Get())
	{
		for (uint32 Handle : BindingHandles)
		{
			InputComponent->RemoveBindingByHandle(Handle);
		}
	}
	BindingHandles.Reset();
	BoundComponent.Reset();
}

void UFeelInputComponent::BindTo(UEnhancedInputComponent* InputComponent)
{
	Unbind();
	if (!InputComponent)
	{
		return;
	}

	BoundComponent = InputComponent;
	for (int32 BindingIndex = 0; BindingIndex < Bindings.Num(); ++BindingIndex)
	{
		const FFeelInputBinding& Binding = Bindings[BindingIndex];
		if (!Binding.Action)
		{
			continue;
		}

		// The play event, plus the end events when the binding ends plays or plays once per press.
		ETriggerEvent Events = Binding.PlayOn;
		if (Binding.bEndWhenInputEnds || Binding.bOncePerPress)
		{
			Events |= ETriggerEvent::Completed | ETriggerEvent::Canceled;
		}
		for (ETriggerEvent Event : { ETriggerEvent::Started, ETriggerEvent::Triggered, ETriggerEvent::Ongoing, ETriggerEvent::Canceled, ETriggerEvent::Completed })
		{
			if (EnumHasAnyFlags(Events, Event))
			{
				const TWeakObjectPtr<UFeelInputComponent> WeakThis(this);
				FEnhancedInputActionEventBinding& EventBinding = InputComponent->BindActionValueLambda(Binding.Action, Event,
					[WeakThis, BindingIndex, Event](const FInputActionValue& Value)
					{
						if (UFeelInputComponent* Component = WeakThis.Get())
						{
							Component->HandleInput(BindingIndex, Event, Value.GetMagnitude());
						}
					});
				BindingHandles.Add(EventBinding.GetHandle());
			}
		}
	}
}

FFeelHandle UFeelInputComponent::GetActiveHandle(int32 BindingIndex) const
{
	return ActiveHandles.IsValidIndex(BindingIndex) ? ActiveHandles[BindingIndex] : FFeelHandle();
}

void UFeelInputComponent::HandleInput(int32 BindingIndex, ETriggerEvent TriggerEvent, float ActionMagnitude)
{
	UWorld* World = GetWorld();
	UFeelSubsystem* Subsystem = World ? World->GetSubsystem<UFeelSubsystem>() : nullptr;
	if (!Subsystem || !Bindings.IsValidIndex(BindingIndex))
	{
		return;
	}
	if (ActiveHandles.Num() != Bindings.Num())
	{
		ActiveHandles.SetNum(Bindings.Num());
		PressConsumed.SetNumZeroed(Bindings.Num());
	}

	const FFeelInputBinding& Binding = Bindings[BindingIndex];
	const bool bInputEnded = TriggerEvent == ETriggerEvent::Completed || TriggerEvent == ETriggerEvent::Canceled;

	// Ending first, so a binding that plays on Completed still starts its new play below.
	if (bInputEnded)
	{
		if (Binding.bEndWhenInputEnds && ActiveHandles[BindingIndex].IsValid())
		{
			const FFeelHandle Handle = ActiveHandles[BindingIndex];
			const UFeelRecipe* Recipe = Binding.Recipe;
			if (Recipe && FFeelPlaybackClock::HasSustain(*Recipe))
			{
				Subsystem->ReleaseFeel(Handle);
			}
			else
			{
				Subsystem->StopFeel(Handle, true);
			}
			ActiveHandles[BindingIndex] = FFeelHandle();
		}
		PressConsumed[BindingIndex] = false;
	}

	if (TriggerEvent != Binding.PlayOn || (Binding.bOncePerPress && PressConsumed[BindingIndex]))
	{
		return;
	}

	const float Magnitude = FMath::Clamp(ActionMagnitude, 0.0f, 1.0f);
	const float PlayIntensity = Binding.bScaleIntensityByValue ? Binding.Intensity * Magnitude : Binding.Intensity;
	FFeelPlayContext Context;
	if (!Binding.ValueParameter.IsNone())
	{
		Context.Parameters.Add(Binding.ValueParameter, ActionMagnitude);
	}

	const FFeelTarget Target = FFeelTarget::FromActor(GetOwner());
	const FFeelHandle Handle = Binding.Recipe
		? Subsystem->PlayFeel(Binding.Recipe, Target, PlayIntensity, Context)
		: (Binding.Event.IsValid() ? Subsystem->SendFeelEvent(Binding.Event, Target, PlayIntensity, Context) : FFeelHandle());

	if (Handle.IsValid())
	{
		ActiveHandles[BindingIndex] = Handle;
		PressConsumed[BindingIndex] = !bInputEnded;
	}
}
