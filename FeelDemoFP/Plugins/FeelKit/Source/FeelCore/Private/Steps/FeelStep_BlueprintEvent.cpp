// Copyright 2026 Billo. All Rights Reserved.

#include "Steps/FeelStep_BlueprintEvent.h"

#include "Engine/LevelScriptActor.h"
#include "Engine/World.h"
#include "FeelTags.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "UObject/UnrealType.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelStep_BlueprintEvent)

DEFINE_LOG_CATEGORY_STATIC(LogFeelSteps, Log, All);

void UFeelStep_BlueprintEvent::OnStart_Implementation(const FFeelContext& Context)
{
	if (!StartEventName.IsNone())
	{
		CallEvent(ResolveReceiver(Context), StartEventName, Context.Intensity);
	}
}

void UFeelStep_BlueprintEvent::OnStop_Implementation(const FFeelContext& Context, bool bInterrupted)
{
	if (!StopEventName.IsNone() && (!bInterrupted || bCallStopEventWhenInterrupted))
	{
		CallEvent(ResolveReceiver(Context), StopEventName, Context.Intensity);
	}
}

FGameplayTag UFeelStep_BlueprintEvent::GetDefaultChannel_Implementation() const
{
	return FeelTags::Meta_Event;
}

bool UFeelStep_BlueprintEvent::SupportsPreview_Implementation() const
{
	return false;
}

#if WITH_EDITOR
void UFeelStep_BlueprintEvent::ValidateStep(TArray<FText>& OutErrors, TArray<FText>& OutWarnings) const
{
	if (StartEventName.IsNone() && StopEventName.IsNone())
	{
		OutWarnings.Add(NSLOCTEXT("FeelKit", "BlueprintEventNoNames", "Blueprint Event has no start or stop event name, so it does nothing."));
	}
}
#endif

bool UFeelStep_BlueprintEvent::CallEvent(UObject* Target, FName EventName, float Intensity)
{
	if (!Target || EventName.IsNone())
	{
		return false;
	}

	UFunction* Function = Target->FindFunction(EventName);
	if (!Function)
	{
		UE_LOG(LogFeelSteps, Warning, TEXT("Blueprint Event: %s has no event or function named %s."), *GetNameSafe(Target), *EventName.ToString());
		return false;
	}

	int32 NumInputs = 0;
	FProperty* FirstInput = nullptr;
	for (TFieldIterator<FProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
	{
		if (It->HasAnyPropertyFlags(CPF_ReturnParm | CPF_OutParm))
		{
			NumInputs = -1;
			break;
		}
		FirstInput = FirstInput ? FirstInput : *It;
		++NumInputs;
	}

	if (NumInputs == 0)
	{
		Target->ProcessEvent(Function, nullptr);
		return true;
	}

	const FDoubleProperty* DoubleInput = NumInputs == 1 ? CastField<FDoubleProperty>(FirstInput) : nullptr;
	const FFloatProperty* FloatInput = NumInputs == 1 ? CastField<FFloatProperty>(FirstInput) : nullptr;
	if (DoubleInput || FloatInput)
	{
		uint8* Parameters = static_cast<uint8*>(FMemory_Alloca(Function->ParmsSize));
		FMemory::Memzero(Parameters, Function->ParmsSize);
		if (DoubleInput)
		{
			DoubleInput->SetPropertyValue_InContainer(Parameters, Intensity);
		}
		else
		{
			FloatInput->SetPropertyValue_InContainer(Parameters, Intensity);
		}
		Target->ProcessEvent(Function, Parameters);
		return true;
	}

	UE_LOG(LogFeelSteps, Warning, TEXT("Blueprint Event: %s.%s must take no inputs or a single float."), *GetNameSafe(Target), *EventName.ToString());
	return false;
}

UObject* UFeelStep_BlueprintEvent::ResolveReceiver(const FFeelContext& Context) const
{
	switch (Receiver)
	{
	case EFeelEventReceiver::PlayerPawn:
		return Context.PlayerController ? Context.PlayerController->GetPawn() : nullptr;
	case EFeelEventReceiver::PlayerController:
		return Context.PlayerController;
	case EFeelEventReceiver::LevelBlueprint:
		return Context.World ? Context.World->GetLevelScriptActor() : nullptr;
	case EFeelEventReceiver::TargetActor:
	default:
		return Context.Target;
	}
}
