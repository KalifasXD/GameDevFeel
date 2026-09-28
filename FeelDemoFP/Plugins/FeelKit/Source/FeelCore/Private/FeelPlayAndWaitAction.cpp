// Copyright 2026 Billo. All Rights Reserved.

#include "FeelPlayAndWaitAction.h"

#include "FeelRecipe.h"
#include "FeelSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelPlayAndWaitAction)

UFeelPlayAndWaitAction* UFeelPlayAndWaitAction::PlayFeelAndWait(UObject* WorldContextObject, UFeelRecipe* Recipe, const FFeelTarget& Target, const FFeelPlayContext& Context, float Intensity)
{
	UFeelPlayAndWaitAction* Action = NewObject<UFeelPlayAndWaitAction>();
	Action->WorldContext = WorldContextObject;
	Action->Recipe = Recipe;
	Action->Target = Target;
	Action->Context = Context;
	Action->Intensity = Intensity;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UFeelPlayAndWaitAction::Activate()
{
	UFeelSubsystem* FoundSubsystem = UFeelSubsystem::Get(WorldContext.Get());
	if (!FoundSubsystem)
	{
		Complete(true);
		return;
	}

	Subsystem = FoundSubsystem;
	FinishedDelegateHandle = FoundSubsystem->OnFeelFinished.AddUObject(this, &UFeelPlayAndWaitAction::HandleFinished);
	Handle = FoundSubsystem->PlayFeel(Recipe, Target, Intensity, Context);
	if (!Handle.IsValid())
	{
		Complete(true);
	}
}

void UFeelPlayAndWaitAction::HandleFinished(FFeelHandle FinishedHandle, UFeelRecipe* FinishedRecipe, bool bInterrupted)
{
	if (FinishedHandle == Handle)
	{
		Complete(bInterrupted);
	}
}

void UFeelPlayAndWaitAction::Complete(bool bCancelled)
{
	if (UFeelSubsystem* BoundSubsystem = Subsystem.Get())
	{
		BoundSubsystem->OnFeelFinished.Remove(FinishedDelegateHandle);
	}
	Subsystem.Reset();

	if (bCancelled)
	{
		OnCancelled.Broadcast(Handle);
	}
	else
	{
		OnFinished.Broadcast(Handle);
	}
	SetReadyToDestroy();
}
