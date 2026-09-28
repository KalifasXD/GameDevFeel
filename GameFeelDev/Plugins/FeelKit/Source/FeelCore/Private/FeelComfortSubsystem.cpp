// Copyright 2026 Billo. All Rights Reserved.

#include "FeelComfortSubsystem.h"

#include "Engine/LocalPlayer.h"
#include "FeelCameraModifier.h"
#include "GameFramework/PlayerController.h"
#include "FeelComfortPreset.h"
#include "FeelComfortSaveGame.h"
#include "FeelSettings.h"
#include "FeelLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelComfortSubsystem)

void UFeelComfortSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Settings are in place before the player's first recipe plays.
	Scales = GetDefault<UFeelSettings>()->DefaultComfortScales;
	LoadComfortSettings();

	// Comfort also reaches engine shakes and vibration, so the player's controller needs FeelKit's camera modifier
	// before any recipe plays.
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		PlayerControllerChangedHandle = LocalPlayer->OnPlayerControllerChanged().AddUObject(this, &UFeelComfortSubsystem::HandlePlayerControllerChanged);
		HandlePlayerControllerChanged(LocalPlayer->PlayerController);
	}
}

void UFeelComfortSubsystem::Deinitialize()
{
	RestoreEngineForceFeedbackScale();
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		LocalPlayer->OnPlayerControllerChanged().Remove(PlayerControllerChangedHandle);
	}
	Super::Deinitialize();
}

void UFeelComfortSubsystem::HandlePlayerControllerChanged(APlayerController* NewPlayerController)
{
	if (!NewPlayerController)
	{
		return;
	}

	const UFeelSettings* Settings = GetDefault<UFeelSettings>();
	if (Settings->bApplyComfortToEngineCameraShakes)
	{
		UFeelCameraModifier::EnsureOn(NewPlayerController);
	}
	ApplyEngineForceFeedbackScale();
}

float UFeelComfortSubsystem::GetEffectiveForceFeedbackScale() const
{
	return GetDefault<UFeelSettings>()->bApplyComfortToEngineForceFeedback
		? FMath::Clamp(Scales.Master * Scales.Haptics, 0.0f, 1.0f)
		: 1.0f;
}

void UFeelComfortSubsystem::ApplyEngineForceFeedbackScale()
{
	const ULocalPlayer* LocalPlayer = GetLocalPlayer();
	APlayerController* PlayerController = LocalPlayer ? LocalPlayer->PlayerController.Get() : nullptr;

	// The controller changed, or the setting was turned off: give the previous controller its own value back.
	if (ScaledController.IsValid() && (ScaledController.Get() != PlayerController || !GetDefault<UFeelSettings>()->bApplyComfortToEngineForceFeedback))
	{
		RestoreEngineForceFeedbackScale();
	}

	if (!PlayerController || !GetDefault<UFeelSettings>()->bApplyComfortToEngineForceFeedback)
	{
		return;
	}

	// Comfort multiplies whatever the game set, instead of replacing it, and only FeelKit's own last value is replaced.
	if (ScaledController.Get() != PlayerController || !FMath::IsNearlyEqual(PlayerController->ForceFeedbackScale, AppliedForceFeedbackScale))
	{
		GameForceFeedbackScale = PlayerController->ForceFeedbackScale;
		ScaledController = PlayerController;
	}

	const float ComfortScale = GetEffectiveForceFeedbackScale();
	AppliedForceFeedbackScale = FMath::Clamp(GameForceFeedbackScale * ComfortScale, 0.0f, 1.0f);
	PlayerController->ForceFeedbackScale = AppliedForceFeedbackScale;

	// Silent controllers are hard to explain, so say it once per player.
	if (AppliedForceFeedbackScale <= UE_KINDA_SMALL_NUMBER)
	{
		if (!bWarnedAboutSilencedHaptics)
		{
			bWarnedAboutSilencedHaptics = true;
			UE_LOG(LogFeel, Warning, TEXT("Controller vibration is off for this player: Haptics comfort %.2f x Master comfort %.2f. Every effect still runs, but no vibration reaches the controller. Set the Haptics comfort scale above 0, or turn off Apply Comfort To Engine Force Feedback in Project Settings > Plugins > FeelKit."),
				Scales.Haptics, Scales.Master);
		}
	}
	else
	{
		bWarnedAboutSilencedHaptics = false;
	}
}

void UFeelComfortSubsystem::RestoreEngineForceFeedbackScale()
{
	if (APlayerController* PlayerController = ScaledController.Get())
	{
		// Only take back what FeelKit wrote; a value the game set since then stays.
		if (FMath::IsNearlyEqual(PlayerController->ForceFeedbackScale, AppliedForceFeedbackScale))
		{
			PlayerController->ForceFeedbackScale = GameForceFeedbackScale;
		}
	}
	ScaledController.Reset();
	GameForceFeedbackScale = 1.0f;
	AppliedForceFeedbackScale = 1.0f;
}

void UFeelComfortSubsystem::SetComfortScales(const FFeelComfortScales& NewScales)
{
	Scales = NewScales;
	Scales.ClampScales();
	HandleScalesChanged();
}

void UFeelComfortSubsystem::SetComfortScalesWithoutSaving(const FFeelComfortScales& NewScales)
{
	Scales = NewScales;
	Scales.ClampScales();
	ApplyEngineForceFeedbackScale();
}

void UFeelComfortSubsystem::SetMasterComfortScale(float Scale)
{
	Scales.Master = FMath::Clamp(Scale, 0.0f, 1.0f);
	HandleScalesChanged();
}

void UFeelComfortSubsystem::SetComfortGroupScale(EFeelComfortGroup Group, float Scale)
{
	Scales.SetGroupScale(Group, Scale);
	HandleScalesChanged();
}

void UFeelComfortSubsystem::ApplyComfortPreset(EFeelBuiltInComfortPreset Preset)
{
	SetComfortScales(GetDefault<UFeelSettings>()->GetPresetScales(Preset));
}

void UFeelComfortSubsystem::ApplyCustomComfortPreset(const UFeelComfortPreset* Preset)
{
	if (Preset)
	{
		SetComfortScales(Preset->Scales);
	}
}

bool UFeelComfortSubsystem::SaveComfortSettings()
{
	return FeelComfort::SaveScales(GetStorage(), GetLocalPlayer(), Scales);
}

bool UFeelComfortSubsystem::LoadComfortSettings()
{
	FFeelComfortScales Loaded = Scales;
	if (!FeelComfort::LoadScales(GetStorage(), GetLocalPlayer(), Loaded))
	{
		return false;
	}
	Loaded.ClampScales();
	Scales = Loaded;
	ApplyEngineForceFeedbackScale();
	return true;
}

void UFeelComfortSubsystem::SetComfortStorage(TScriptInterface<IFeelComfortStorage> NewStorage)
{
	Storage = NewStorage.GetObject();
	LoadComfortSettings();
}

UObject* UFeelComfortSubsystem::GetStorage()
{
	if (!Storage)
	{
		UClass* StorageClass = GetDefault<UFeelSettings>()->ComfortStorageClass.LoadSynchronous();
		if (!StorageClass || !StorageClass->ImplementsInterface(UFeelComfortStorage::StaticClass()))
		{
			StorageClass = UFeelSaveGameComfortStorage::StaticClass();
		}
		Storage = NewObject<UObject>(this, StorageClass);
	}
	return Storage;
}

void UFeelComfortSubsystem::HandleScalesChanged()
{
	ApplyEngineForceFeedbackScale();
	if (GetDefault<UFeelSettings>()->bAutoSaveComfort)
	{
		SaveComfortSettings();
	}
}
