// Copyright 2026 Billo. All Rights Reserved.

#include "FeelSwitch.h"

#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "FeelBlueprintLibrary.h"
#include "FeelComfortMenu.h"
#include "FeelLog.h"
#include "FeelSwitchOverlay.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelSwitch)

#define LOCTEXT_NAMESPACE "FeelSwitch"

namespace FeelSwitchTiming
{
	constexpr double CardFadeSeconds = 0.5;
	/** Keys pressed this soon after the level starts (still held from loading) do not dismiss the start card. */
	constexpr double CardDismissDelay = 0.5;
	constexpr double PulseRiseSeconds = 0.08;
	constexpr double PulseFallSeconds = 0.5;
	/** Draw order of the display among other viewport widgets: above game HUD widgets added at the default 0. */
	constexpr int32 OverlayZOrder = 50;
}

namespace
{
	/** "Tab / View / Share": the first keyboard key and the first controller button of a list, for on-screen text. */
	FText KeysLabel(const TArray<FKey>& Keys)
	{
		const FKey* Keyboard = Keys.FindByPredicate([](const FKey& Key) { return Key.IsValid() && !Key.IsGamepadKey(); });
		const FKey* Gamepad = Keys.FindByPredicate([](const FKey& Key) { return Key.IsValid() && Key.IsGamepadKey(); });
		if (Keyboard && Gamepad)
		{
			return FText::Format(NSLOCTEXT("FeelSwitch", "BadgeHintBoth", "{0} / {1}"), AFeelSwitch::GetKeyLabel(*Keyboard), AFeelSwitch::GetKeyLabel(*Gamepad));
		}
		if (Keyboard || Gamepad)
		{
			return AFeelSwitch::GetKeyLabel(Keyboard ? *Keyboard : *Gamepad);
		}
		return FText::GetEmpty();
	}
}

AFeelSwitch::AFeelSwitch()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	// After player controllers have read this frame's key presses.
	PrimaryActorTick.TickGroup = TG_PostPhysics;
	PrimaryActorTick.bTickEvenWhenPaused = true;

	SwitchKeys = { EKeys::Tab, EKeys::Gamepad_Special_Left };
	ComfortMenuKeys = { EKeys::O, EKeys::Gamepad_Special_Right };
	ComfortMenuControlName = LOCTEXT("ComfortMenuControl", "Comfort settings");
	StartCardTitle = LOCTEXT("DefaultTitle", "FeelKit");
	StartCardText = LOCTEXT("DefaultText", "Everything you see, hear and feel on top of the game comes from FeelKit.");

	SetCanBeDamaged(false);
	bReplicates = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AFeelSwitch::BeginPlay()
{
	Super::BeginPlay();

	for (TActorIterator<AFeelSwitch> It(GetWorld()); It; ++It)
	{
		if (*It != this && It->HasActorBegunPlay())
		{
			UE_LOG(LogFeel, Warning, TEXT("%s: this level already has a Feel Switch (%s), so this one does nothing. Keep one Feel Switch per level."), *GetName(), *It->GetName());
			SetActorTickEnabled(false);
			return;
		}
	}

	bRestoreEnabledOnEnd = UFeelBlueprintLibrary::IsFeelEnabled();
	bOwnsSwitch = true;
	UFeelBlueprintLibrary::SetFeelEnabled(bStartEnabled);
	bLastEnabled = bStartEnabled;
	StartTime = FPlatformTime::Seconds();
}

void AFeelSwitch::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RemoveOverlays();
	if (bOwnsSwitch)
	{
		// The switch lives for the whole program (in the editor, across play sessions): leave it as it was found.
		UFeelBlueprintLibrary::SetFeelEnabled(bRestoreEnabledOnEnd);
		bOwnsSwitch = false;
	}
	Super::EndPlay(EndPlayReason);
}

void AFeelSwitch::Switch()
{
	UFeelBlueprintLibrary::ToggleFeel();
}

void AFeelSwitch::ToggleComfortMenu(APlayerController* PlayerController)
{
	if (UFeelComfortMenu* Open = GetOpenComfortMenu(PlayerController))
	{
		Open->CloseMenu();
		return;
	}

	UFeelComfortMenu* Menu = UFeelComfortMenu::ShowFeelComfortMenu(PlayerController, ComfortMenuClass, bPauseInComfortMenu);
	if (Menu)
	{
		// The key that opened the menu also closes it; the menu has keyboard focus, so it reads the key itself.
		for (const FKey& Key : ComfortMenuKeys)
		{
			if (Key.IsValid())
			{
				Menu->CloseKeys.AddUnique(Key);
			}
		}
		Menu->RefreshCloseHint();
		ComfortMenus.RemoveAll([](const TWeakObjectPtr<UFeelComfortMenu>& Entry) { return !Entry.IsValid() || !Entry->IsInViewport(); });
		ComfortMenus.Add(Menu);
	}
}

UFeelComfortMenu* AFeelSwitch::GetOpenComfortMenu(const APlayerController* PlayerController) const
{
	for (const TWeakObjectPtr<UFeelComfortMenu>& Entry : ComfortMenus)
	{
		UFeelComfortMenu* Menu = Entry.Get();
		if (Menu && Menu->IsInViewport() && Menu->GetOwningPlayer() == PlayerController)
		{
			return Menu;
		}
	}
	return nullptr;
}

FText AFeelSwitch::GetKeyLabel(const FKey& Key)
{
	if (Key == EKeys::Gamepad_Special_Left)
	{
		return LOCTEXT("ViewShare", "View / Share");
	}
	if (Key == EKeys::Gamepad_Special_Right)
	{
		return LOCTEXT("MenuOptions", "Menu / Options");
	}
	return Key.GetDisplayName(false);
}

FText AFeelSwitch::GetSwitchHint() const
{
	const FKey* Keyboard = SwitchKeys.FindByPredicate([](const FKey& Key) { return Key.IsValid() && !Key.IsGamepadKey(); });
	const FKey* Gamepad = SwitchKeys.FindByPredicate([](const FKey& Key) { return Key.IsValid() && Key.IsGamepadKey(); });
	if (Keyboard && Gamepad)
	{
		return FText::Format(LOCTEXT("HintBoth", "Press {0} (or {1} on a controller) to turn the feel off and on."), GetKeyLabel(*Keyboard), GetKeyLabel(*Gamepad));
	}
	if (Keyboard || Gamepad)
	{
		return FText::Format(LOCTEXT("HintOne", "Press {0} to turn the feel off and on."), GetKeyLabel(Keyboard ? *Keyboard : *Gamepad));
	}
	return LOCTEXT("HintNone", "Add a key to Switch Keys on the Feel Switch to turn the feel off and on.");
}

bool AFeelSwitch::IsStartCardVisible() const
{
	for (const FPlayerOverlay& Overlay : Overlays)
	{
		if (Overlay.Widget.IsValid() && Overlay.Widget->GetCardOpacity() > 0.0f)
		{
			return true;
		}
	}
	return false;
}

int32 AFeelSwitch::GetShownControlRows() const
{
	for (const FPlayerOverlay& Overlay : Overlays)
	{
		if (Overlay.Widget.IsValid())
		{
			return Overlay.Widget->GetControlRowCount();
		}
	}
	return 0;
}

void AFeelSwitch::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const double Now = FPlatformTime::Seconds();

	bSwitchedThisFrame = false;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* Controller = It->Get();
		if (Controller && Controller->IsLocalController())
		{
			ReadPlayerKeys(*Controller);
		}
	}

	// Also notices switches from Blueprint nodes and the console variable.
	const bool bEnabled = UFeelBlueprintLibrary::IsFeelEnabled();
	if (bEnabled != bLastEnabled)
	{
		bLastEnabled = bEnabled;
		LastSwitchTime = Now;
		OnFeelSwitched.Broadcast(bEnabled);
	}

	UpdateOverlays();
}

void AFeelSwitch::ReadPlayerKeys(APlayerController& Controller)
{
	UPlayerInput* Input = Controller.PlayerInput;
	if (!Input)
	{
		return;
	}

	for (const FKey& Key : SwitchKeys)
	{
		if (Key.IsValid() && Input->WasJustPressed(Key))
		{
			if (!bSwitchedThisFrame)
			{
				Switch();
				bSwitchedThisFrame = true;
			}
			if (!bStartCardDismissed)
			{
				bStartCardDismissed = true;
				DismissTime = FPlatformTime::Seconds();
			}
			break;
		}
	}

	for (const FKey& Key : ComfortMenuKeys)
	{
		if (Key.IsValid() && Input->WasJustPressed(Key))
		{
			ToggleComfortMenu(&Controller);
			break;
		}
	}

	if (!bStartCardDismissed && FPlatformTime::Seconds() - StartTime > FeelSwitchTiming::CardDismissDelay)
	{
		// Every button on keyboard, mouse and controller (sticks and mouse movement do not count).
		static const TArray<FKey> Buttons = []()
		{
			TArray<FKey> AllKeys;
			EKeys::GetAllKeys(AllKeys);
			AllKeys.RemoveAll([](const FKey& Key) { return !Key.IsDigital() || Key.IsTouch() || Key.IsGesture(); });
			return AllKeys;
		}();
		for (const FKey& Key : Buttons)
		{
			if (Input->WasJustPressed(Key))
			{
				bStartCardDismissed = true;
				DismissTime = FPlatformTime::Seconds();
				break;
			}
		}
	}
}

void AFeelSwitch::UpdateOverlays()
{
	UWorld* World = GetWorld();
	UGameViewportClient* Viewport = World ? World->GetGameViewport() : nullptr;
	UGameInstance* GameInstance = GetGameInstance();
	if (!bShowBuiltInDisplay || !Viewport || !GameInstance || !bOwnsSwitch || IsNetMode(NM_DedicatedServer))
	{
		RemoveOverlays();
		return;
	}

	// One display per local player (split screen gets one in each player's part of the screen).
	Overlays.RemoveAll([](const FPlayerOverlay& Overlay) { return !Overlay.Player.IsValid(); });
	for (ULocalPlayer* Player : GameInstance->GetLocalPlayers())
	{
		if (Player && !Overlays.ContainsByPredicate([Player](const FPlayerOverlay& Overlay) { return Overlay.Player.Get() == Player; }))
		{
			const FText BadgeHint = KeysLabel(SwitchKeys);

			TArray<TPair<FText, FText>> ControlRows;
			for (const FFeelSwitchControl& Control : Controls)
			{
				if (!Control.Action.IsEmpty() || !Control.Keys.IsEmpty())
				{
					ControlRows.Emplace(Control.Action, Control.Keys);
				}
			}
			const FText ComfortMenuKeysLabel = KeysLabel(ComfortMenuKeys);
			if (!ComfortMenuKeysLabel.IsEmpty())
			{
				ControlRows.Emplace(ComfortMenuControlName, ComfortMenuKeysLabel);
			}

			FPlayerOverlay& Overlay = Overlays.AddDefaulted_GetRef();
			Overlay.Player = Player;
			Overlay.Widget = SNew(SFeelSwitchOverlay)
				.Title(StartCardTitle)
				.Text(StartCardText)
				.Hint(GetSwitchHint())
				.BadgeHint(BadgeHint)
				.Controls(ControlRows);
			Viewport->AddViewportWidgetForPlayer(Player, Overlay.Widget.ToSharedRef(), FeelSwitchTiming::OverlayZOrder);
		}
	}

	// Start card: full until it times out or a key dismisses it, then fades.
	const double Now = FPlatformTime::Seconds();
	float CardOpacity = 0.0f;
	if (bShowStartCard)
	{
		const double FadeStart = bStartCardDismissed ? FMath::Min(DismissTime, StartTime + StartCardSeconds) : StartTime + StartCardSeconds;
		CardOpacity = static_cast<float>(FMath::Clamp(1.0 - (Now - FadeStart) / FeelSwitchTiming::CardFadeSeconds, 0.0, 1.0));
	}

	// Switch confirmation on the badge: a quick rise, then an eased settle.
	const double SinceSwitch = Now - LastSwitchTime;
	float Pulse = 0.0f;
	if (SinceSwitch < FeelSwitchTiming::PulseRiseSeconds)
	{
		Pulse = static_cast<float>(SinceSwitch / FeelSwitchTiming::PulseRiseSeconds);
	}
	else if (SinceSwitch < FeelSwitchTiming::PulseRiseSeconds + FeelSwitchTiming::PulseFallSeconds)
	{
		const float Fall = static_cast<float>((SinceSwitch - FeelSwitchTiming::PulseRiseSeconds) / FeelSwitchTiming::PulseFallSeconds);
		Pulse = FMath::Square(1.0f - Fall);
	}

	for (const FPlayerOverlay& Overlay : Overlays)
	{
		Overlay.Widget->SetFeelEnabled(bLastEnabled);
		Overlay.Widget->SetCardOpacity(CardOpacity);
		Overlay.Widget->SetBadgePulse(Pulse);
	}
}

void AFeelSwitch::RemoveOverlays()
{
	UWorld* World = GetWorld();
	UGameViewportClient* Viewport = World ? World->GetGameViewport() : nullptr;
	for (const FPlayerOverlay& Overlay : Overlays)
	{
		if (Viewport && Overlay.Player.IsValid() && Overlay.Widget.IsValid())
		{
			Viewport->RemoveViewportWidgetForPlayer(Overlay.Player.Get(), Overlay.Widget.ToSharedRef());
		}
	}
	Overlays.Reset();
}

#undef LOCTEXT_NAMESPACE
