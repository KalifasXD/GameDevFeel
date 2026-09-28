// Copyright 2026 Billo. All Rights Reserved.

#include "FeelBlueprintLibrary.h"

#include "Engine/LocalPlayer.h"
#include "FeelComfortSubsystem.h"
#include "FeelSettings.h"
#include "FeelSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelBlueprintLibrary)

FFeelHandle UFeelBlueprintLibrary::PlayFeel(const UObject* WorldContextObject, UFeelRecipe* Recipe, const FFeelTarget& Target, float Intensity)
{
	UFeelSubsystem* Subsystem = UFeelSubsystem::Get(WorldContextObject);
	return Subsystem ? Subsystem->PlayFeel(Recipe, Target, Intensity) : FFeelHandle();
}

// FEELKIT_PRO_BEGIN
FFeelHandle UFeelBlueprintLibrary::PlayFeelWithContext(const UObject* WorldContextObject, UFeelRecipe* Recipe, const FFeelTarget& Target, const FFeelPlayContext& Context, float Intensity)
{
	UFeelSubsystem* Subsystem = UFeelSubsystem::Get(WorldContextObject);
	return Subsystem ? Subsystem->PlayFeel(Recipe, Target, Intensity, Context) : FFeelHandle();
}

FFeelHandle UFeelBlueprintLibrary::SendFeelEvent(const UObject* WorldContextObject, FGameplayTag Event, const FFeelTarget& Target, const FFeelPlayContext& Context, float Intensity)
{
	UFeelSubsystem* Subsystem = UFeelSubsystem::Get(WorldContextObject);
	return Subsystem ? Subsystem->SendFeelEvent(Event, Target, Intensity, Context) : FFeelHandle();
}

void UFeelBlueprintLibrary::ReleaseFeel(const UObject* WorldContextObject, FFeelHandle Handle)
{
	if (UFeelSubsystem* Subsystem = UFeelSubsystem::Get(WorldContextObject))
	{
		Subsystem->ReleaseFeel(Handle);
	}
}

bool UFeelBlueprintLibrary::SetFeelParameter(const UObject* WorldContextObject, FFeelHandle Handle, FName ParameterName, float Value)
{
	UFeelSubsystem* Subsystem = UFeelSubsystem::Get(WorldContextObject);
	return Subsystem && Subsystem->SetFeelParameter(Handle, ParameterName, Value);
}

void UFeelBlueprintLibrary::AddToFeelAccumulator(const UObject* WorldContextObject, FName AccumulatorName, float Amount, AActor* Actor)
{
	if (UFeelSubsystem* Subsystem = UFeelSubsystem::Get(WorldContextObject))
	{
		Subsystem->AddToAccumulator(AccumulatorName, Amount, Actor);
	}
}

void UFeelBlueprintLibrary::SetFeelAccumulator(const UObject* WorldContextObject, FName AccumulatorName, float Value, AActor* Actor)
{
	if (UFeelSubsystem* Subsystem = UFeelSubsystem::Get(WorldContextObject))
	{
		Subsystem->SetAccumulator(AccumulatorName, Value, Actor);
	}
}

float UFeelBlueprintLibrary::GetFeelAccumulator(const UObject* WorldContextObject, FName AccumulatorName, AActor* Actor)
{
	const UFeelSubsystem* Subsystem = UFeelSubsystem::Get(WorldContextObject);
	return Subsystem ? Subsystem->GetAccumulator(AccumulatorName, Actor) : 0.0f;
}
// FEELKIT_PRO_END

void UFeelBlueprintLibrary::StopFeel(const UObject* WorldContextObject, FFeelHandle Handle, bool bBlendOut)
{
	if (UFeelSubsystem* Subsystem = UFeelSubsystem::Get(WorldContextObject))
	{
		Subsystem->StopFeel(Handle, bBlendOut);
	}
}

void UFeelBlueprintLibrary::StopAllFeel(const UObject* WorldContextObject, AActor* Target)
{
	if (UFeelSubsystem* Subsystem = UFeelSubsystem::Get(WorldContextObject))
	{
		Subsystem->StopAllFeel(Target);
	}
}

bool UFeelBlueprintLibrary::IsFeelPlaying(const UObject* WorldContextObject, FFeelHandle Handle)
{
	const UFeelSubsystem* Subsystem = UFeelSubsystem::Get(WorldContextObject);
	return Subsystem && Subsystem->IsPlaying(Handle);
}

bool UFeelBlueprintLibrary::IsValidFeelHandle(FFeelHandle Handle)
{
	return Handle.IsValid();
}

FFeelTarget UFeelBlueprintLibrary::MakeFeelTargetFromActor(AActor* Actor)
{
	return FFeelTarget::FromActor(Actor);
}

FFeelTarget UFeelBlueprintLibrary::MakeFeelTargetFromComponent(USceneComponent* Component)
{
	return FFeelTarget::FromComponent(Component);
}

FFeelTarget UFeelBlueprintLibrary::MakeFeelTargetAtLocation(FVector Location)
{
	return FFeelTarget::AtLocation(Location);
}

FFeelTarget UFeelBlueprintLibrary::MakeFeelTargetFromLocalPlayerCamera(int32 PlayerIndex)
{
	return FFeelTarget::FromLocalPlayerCamera(PlayerIndex);
}

FFeelTarget UFeelBlueprintLibrary::MakeFeelTargetFromWidget(UWidget* Widget)
{
	return FFeelTarget::FromWidget(Widget);
}

UFeelComfortSubsystem* UFeelBlueprintLibrary::GetFeelComfort(const APlayerController* PlayerController)
{
	const ULocalPlayer* LocalPlayer = PlayerController ? PlayerController->GetLocalPlayer() : nullptr;
	return LocalPlayer ? LocalPlayer->GetSubsystem<UFeelComfortSubsystem>() : nullptr;
}

// FEELKIT_PRO_BEGIN
TArray<FName> UFeelBlueprintLibrary::GetFeelAccumulatorOptions()
{
	TArray<FName> Names;
	for (const FFeelAccumulatorDefinition& Definition : GetDefault<UFeelSettings>()->Accumulators)
	{
		if (!Definition.Name.IsNone())
		{
			Names.AddUnique(Definition.Name);
		}
	}
	return Names;
}
// FEELKIT_PRO_END

namespace FeelSwitchPrivate
{
	IConsoleVariable* FeelEnabledVariable()
	{
		static IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(TEXT("feel.Enabled"));
		return Variable;
	}
}

void UFeelBlueprintLibrary::SetFeelEnabled(bool bEnabled)
{
	IConsoleVariable* Variable = FeelSwitchPrivate::FeelEnabledVariable();
	if (Variable && Variable->GetBool() != bEnabled)
	{
		// Set at the variable's current priority, so it also works after feel.Enabled was typed in the console.
		Variable->Set(bEnabled ? TEXT("1") : TEXT("0"), static_cast<EConsoleVariableFlags>(Variable->GetFlags() & ECVF_SetByMask));
	}
}

bool UFeelBlueprintLibrary::ToggleFeel()
{
	SetFeelEnabled(!IsFeelEnabled());
	return IsFeelEnabled();
}

bool UFeelBlueprintLibrary::IsFeelEnabled()
{
	const IConsoleVariable* Variable = FeelSwitchPrivate::FeelEnabledVariable();
	return !Variable || Variable->GetBool();
}
