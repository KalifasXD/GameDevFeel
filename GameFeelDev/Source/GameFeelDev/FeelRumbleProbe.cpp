// Development probe for controller rumble. Lives in the host project, not in the plugin.
// Usage in Play In Editor: open the console and type feeltest.rumble, hold the controller, and note which stage rumbles.
// Stage 1 asks Windows (XInput) directly, stage 2 goes through Unreal's force feedback, stage 3 through FeelKit (R_Haptic).
// Editor builds on Windows only.

#if WITH_EDITOR && PLATFORM_WINDOWS

#include "Containers/Ticker.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "FeelComfortSubsystem.h"
#include "FeelRecipe.h"
#include "FeelSubsystem.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/InputDeviceSubsystem.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformProcess.h"

DEFINE_LOG_CATEGORY_STATIC(LogFeelRumbleProbe, Log, All);

namespace FeelRumbleProbe
{
	// Layouts of XINPUT_STATE and XINPUT_VIBRATION, so no Windows headers are needed.
	struct FXInputGamepad
	{
		uint16 Buttons;
		uint8 LeftTrigger;
		uint8 RightTrigger;
		int16 ThumbLX;
		int16 ThumbLY;
		int16 ThumbRX;
		int16 ThumbRY;
	};

	struct FXInputState
	{
		uint32 PacketNumber;
		FXInputGamepad Gamepad;
	};

	struct FXInputVibration
	{
		uint16 LeftMotorSpeed;
		uint16 RightMotorSpeed;
	};

	using FGetState = uint32(__stdcall*)(uint32, FXInputState*);
	using FSetState = uint32(__stdcall*)(uint32, FXInputVibration*);

	FGetState GetState = nullptr;
	FSetState SetState = nullptr;

	bool LoadXInput()
	{
		if (GetState && SetState)
		{
			return true;
		}
		if (void* Dll = FPlatformProcess::GetDllHandle(TEXT("xinput1_4.dll")))
		{
			GetState = static_cast<FGetState>(FPlatformProcess::GetDllExport(Dll, TEXT("XInputGetState")));
			SetState = static_cast<FSetState>(FPlatformProcess::GetDllExport(Dll, TEXT("XInputSetState")));
		}
		return GetState && SetState;
	}

	void Say(const FString& Line, const FColor& Color = FColor::Cyan)
	{
		UE_LOG(LogFeelRumbleProbe, Display, TEXT("%s"), *Line);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 20.0f, Color, Line);
		}
	}

	TArray<uint32> ConnectedSlots()
	{
		TArray<uint32> Slots;
		if (LoadXInput())
		{
			for (uint32 Slot = 0; Slot < 4; ++Slot)
			{
				FXInputState State = {};
				if (GetState(Slot, &State) == 0)
				{
					Slots.Add(Slot);
				}
			}
		}
		return Slots;
	}

	void SetAllMotors(float Strength)
	{
		const uint16 Speed = static_cast<uint16>(FMath::Clamp(Strength, 0.0f, 1.0f) * 65535.0f);
		for (uint32 Slot : ConnectedSlots())
		{
			FXInputVibration Vibration = { Speed, Speed };
			SetState(Slot, &Vibration);
		}
	}

	APlayerController* FindPlayer()
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if ((Context.WorldType == EWorldType::PIE || Context.WorldType == EWorldType::Game) && Context.World())
			{
				if (APlayerController* Controller = Context.World()->GetFirstPlayerController())
				{
					return Controller;
				}
			}
		}
		return nullptr;
	}

	FString DeviceText(const FHardwareDeviceIdentifier& Device)
	{
		return FString::Printf(TEXT("%s / %s"), *Device.InputClassName.ToString(), *Device.HardwareDeviceIdentifier.ToString());
	}

	void Report()
	{
		const TArray<uint32> Slots = ConnectedSlots();
		Say(FString::Printf(TEXT("[1] Windows XInput: %s"), !LoadXInput() ? TEXT("xinput1_4.dll not available")
			: Slots.Num() == 0 ? TEXT("NO XInput controller connected (Unreal can only rumble XInput controllers)")
			: *FString::Printf(TEXT("%d XInput controller(s) connected, first slot %u"), Slots.Num(), Slots[0])));

		APlayerController* Controller = FindPlayer();
		if (!Controller)
		{
			Say(TEXT("No player controller: start Play In Editor first."), FColor::Red);
			return;
		}

		const FPlatformUserId User = Controller->GetPlatformUserId();
		Say(FString::Printf(TEXT("[2] Player: controller id %d, force feedback enabled %d, scale %.2f, window focused %d"),
			Controller->GetLocalPlayer() ? Controller->GetLocalPlayer()->GetControllerId() : -1, Controller->bForceFeedbackEnabled ? 1 : 0, Controller->ForceFeedbackScale,
			FSlateApplication::IsInitialized() && FSlateApplication::Get().GetActiveTopLevelWindow().IsValid() ? 1 : 0));

		if (const UInputDeviceSubsystem* Devices = UInputDeviceSubsystem::Get())
		{
			const FInputDeviceId Gamepad = Devices->GetLatestDeviceOfType(User, EHardwareDevicePrimaryType::Gamepad);
			Say(FString::Printf(TEXT("[2] Last device used: %s. Last gamepad: %s"),
				*DeviceText(Devices->GetMostRecentlyUsedHardwareDevice(User)),
				Gamepad.IsValid() ? *DeviceText(Devices->GetInputDeviceHardwareIdentifier(Gamepad)) : TEXT("none yet (move a stick first)")));
		}

		if (const ULocalPlayer* LocalPlayer = Controller->GetLocalPlayer())
		{
			if (const UFeelComfortSubsystem* Comfort = LocalPlayer->GetSubsystem<UFeelComfortSubsystem>())
			{
				Say(FString::Printf(TEXT("[3] FeelKit comfort: master %.2f, haptics %.2f"), Comfort->GetComfortScalesRef().Master, Comfort->GetComfortScalesRef().Haptics));
			}
		}
	}

	void Run()
	{
		Say(TEXT("Rumble probe: hold the controller. Stage 1 in 2 s, stage 2 in 6 s, stage 3 in 10 s."), FColor::Yellow);
		Report();

		auto After = [](float Seconds, TFunction<void()> Action)
		{
			FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Action](float)
			{
				Action();
				return false;
			}), Seconds);
		};

		After(2.0f, []()
		{
			const int32 Slots = ConnectedSlots().Num();
			Say(FString::Printf(TEXT("STAGE 1: Windows XInput directly, full strength for 1.5 s on %d controller(s)"), Slots), FColor::Yellow);
			SetAllMotors(1.0f);
		});
		After(3.5f, []() { SetAllMotors(0.0f); });

		After(6.0f, []()
		{
			if (APlayerController* Controller = FindPlayer())
			{
				Say(TEXT("STAGE 2: Unreal Play Dynamic Force Feedback, full strength for 1.5 s"), FColor::Yellow);
				Controller->PlayDynamicForceFeedback(1.0f, 1.5f, true, true, true, true);
			}
		});
		After(6.5f, []()
		{
			if (APlayerController* Controller = FindPlayer())
			{
				const FForceFeedbackValues& Values = Controller->ForceFeedbackValues;
				Say(FString::Printf(TEXT("STAGE 2 motor values: %.2f %.2f %.2f %.2f"), Values.LeftLarge, Values.LeftSmall, Values.RightLarge, Values.RightSmall));
			}
		});

		After(10.0f, []()
		{
			APlayerController* Controller = FindPlayer();
			UFeelRecipe* Recipe = LoadObject<UFeelRecipe>(nullptr, TEXT("/Game/FeelKitTests/R_Haptic.R_Haptic"));
			UFeelSubsystem* Subsystem = Controller ? Controller->GetWorld()->GetSubsystem<UFeelSubsystem>() : nullptr;
			if (!Recipe || !Subsystem)
			{
				Say(TEXT("STAGE 3 skipped: R_Haptic or the FeelKit subsystem not found"), FColor::Red);
				return;
			}
			const FFeelHandle Handle = Subsystem->PlayFeel(Recipe, FFeelTarget::FromActor(Controller->GetPawn()));
			Say(FString::Printf(TEXT("STAGE 3: FeelKit R_Haptic (%s)"), Handle.IsValid() ? TEXT("playing") : TEXT("did not start")), FColor::Yellow);
		});
		After(10.5f, []()
		{
			if (APlayerController* Controller = FindPlayer())
			{
				const FForceFeedbackValues& Values = Controller->ForceFeedbackValues;
				Say(FString::Printf(TEXT("STAGE 3 motor values: %.2f %.2f %.2f %.2f"), Values.LeftLarge, Values.LeftSmall, Values.RightLarge, Values.RightSmall));
			}
		});
		After(13.0f, []()
		{
			Report();
			Say(TEXT("Rumble probe done. Which stages rumbled: 1, 2, 3?"), FColor::Yellow);
		});
	}

	FAutoConsoleCommand Command(
		TEXT("feeltest.rumble"),
		TEXT("Tests controller rumble layer by layer: Windows XInput directly, Unreal force feedback, FeelKit R_Haptic."),
		FConsoleCommandDelegate::CreateStatic(&Run));
}

#endif // WITH_EDITOR && PLATFORM_WINDOWS
