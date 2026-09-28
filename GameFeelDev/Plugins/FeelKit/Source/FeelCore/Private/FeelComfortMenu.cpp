// Copyright 2026 Billo. All Rights Reserved.

#include "FeelComfortMenu.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "FeelBlueprintLibrary.h"
#include "FeelComfortSubsystem.h"
#include "FeelLog.h"
#include "FeelRecipe.h"
#include "FeelSettings.h"
#include "FeelSubsystem.h"
#include "FeelTypes.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelComfortMenu)

#define LOCTEXT_NAMESPACE "FeelComfortMenu"

UFeelComfortMenu::UFeelComfortMenu(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);
	CloseKeys = { EKeys::Escape, EKeys::Gamepad_FaceButton_Right, EKeys::Gamepad_Special_Right };

	Descriptions.Add(EFeelComfortMenuRow::Master, LOCTEXT("MasterDescription", "Scales every effect below at once."));
	Descriptions.Add(EFeelComfortMenuRow::CameraShake, LOCTEXT("CameraShakeDescription", "How much the camera shakes, for example on hits and explosions."));
	Descriptions.Add(EFeelComfortMenuRow::CameraMotion, LOCTEXT("CameraMotionDescription", "Camera pushes, zooms and tilts that follow the action."));
	Descriptions.Add(EFeelComfortMenuRow::Flashes, LOCTEXT("FlashesDescription", "Bright flashes over the whole screen and flashing lights."));
	Descriptions.Add(EFeelComfortMenuRow::HitstopAndSlowMo, LOCTEXT("HitstopDescription", "Short freezes on impacts and slow-motion moments."));
	Descriptions.Add(EFeelComfortMenuRow::ScreenDistortion, LOCTEXT("ScreenDistortionDescription", "Darkened edges, color fringes and color changes over the picture."));
	Descriptions.Add(EFeelComfortMenuRow::Haptics, LOCTEXT("HapticsDescription", "Controller vibration."));
	Descriptions.Add(EFeelComfortMenuRow::CameraRoll, LOCTEXT("CameraRollDescription", "Lets the camera tilt sideways. Turn it off if the tilt makes you uncomfortable."));
	Descriptions.Add(EFeelComfortMenuRow::FieldOfViewSpeed, LOCTEXT("ZoomSpeedDescription", "Limits how fast the view may zoom in or out. No limit keeps every zoom as designed."));
	Descriptions.Add(EFeelComfortMenuRow::FlashLimiter, LOCTEXT("FlashLimiterDescription", "Softens flashes when many happen in a short time."));
	Descriptions.Add(EFeelComfortMenuRow::Presets, LOCTEXT("PresetsDescription", "Sets every option at once. You can adjust them afterwards."));
	Descriptions.Add(EFeelComfortMenuRow::Try, LOCTEXT("TryDescription", "Plays a sample of the effects with your current settings."));

	NoLimitText = LOCTEXT("NoLimit", "No limit");
	SavedText = LOCTEXT("Saved", "Saved");
	UnsavedText = LOCTEXT("Unsaved", "Saving...");
}

UFeelComfortMenu* UFeelComfortMenu::ShowFeelComfortMenu(APlayerController* PlayerController, TSubclassOf<UFeelComfortMenu> MenuClass, bool bPauseGame, int32 ZOrder)
{
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		UE_LOG(LogFeel, Warning, TEXT("Show Feel Comfort Menu needs a local player controller."));
		return nullptr;
	}

	UClass* Class = MenuClass.Get();
	if (!Class)
	{
		Class = GetDefault<UFeelSettings>()->ComfortMenuClass.TryLoadClass<UFeelComfortMenu>();
	}
	if (!Class || Class->HasAnyClassFlags(CLASS_Abstract))
	{
		UE_LOG(LogFeel, Warning, TEXT("Show Feel Comfort Menu: no comfort menu to open. Set Comfort Menu Class in Project Settings > Plugins > FeelKit, or pass a menu class to the node."));
		return nullptr;
	}

	UFeelComfortMenu* Menu = CreateWidget<UFeelComfortMenu>(PlayerController, Class);
	if (!Menu)
	{
		return nullptr;
	}

	Menu->bOwnsInputMode = true;
	Menu->bPreviousShowMouseCursor = PlayerController->bShowMouseCursor;
	Menu->AddToViewport(ZOrder);

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(Menu->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PlayerController->SetInputMode(InputMode);
	PlayerController->bShowMouseCursor = true;

	if (bPauseGame && !UGameplayStatics::IsGamePaused(PlayerController))
	{
		Menu->bPausedGame = UGameplayStatics::SetGamePaused(PlayerController, true);
	}

	// Keyboard and gamepad start on the first control.
	if (Menu->MasterSlider)
	{
		Menu->MasterSlider->SetUserFocus(PlayerController);
	}
	return Menu;
}

void UFeelComfortMenu::NativeConstruct()
{
	Super::NativeConstruct();

	if (MasterSlider) { MasterSlider->OnValueChanged.AddUniqueDynamic(this, &UFeelComfortMenu::HandleMasterChanged); }
	if (CameraShakeSlider) { CameraShakeSlider->OnValueChanged.AddUniqueDynamic(this, &UFeelComfortMenu::HandleCameraShakeChanged); }
	if (CameraMotionSlider) { CameraMotionSlider->OnValueChanged.AddUniqueDynamic(this, &UFeelComfortMenu::HandleCameraMotionChanged); }
	if (FlashesSlider) { FlashesSlider->OnValueChanged.AddUniqueDynamic(this, &UFeelComfortMenu::HandleFlashesChanged); }
	if (HitstopSlider) { HitstopSlider->OnValueChanged.AddUniqueDynamic(this, &UFeelComfortMenu::HandleHitstopChanged); }
	if (ScreenDistortionSlider) { ScreenDistortionSlider->OnValueChanged.AddUniqueDynamic(this, &UFeelComfortMenu::HandleScreenDistortionChanged); }
	if (HapticsSlider) { HapticsSlider->OnValueChanged.AddUniqueDynamic(this, &UFeelComfortMenu::HandleHapticsChanged); }
	if (ZoomSpeedSlider) { ZoomSpeedSlider->OnValueChanged.AddUniqueDynamic(this, &UFeelComfortMenu::HandleZoomSpeedChanged); }
	if (CameraRollCheckBox) { CameraRollCheckBox->OnCheckStateChanged.AddUniqueDynamic(this, &UFeelComfortMenu::HandleCameraRollChanged); }
	if (FlashLimiterCheckBox) { FlashLimiterCheckBox->OnCheckStateChanged.AddUniqueDynamic(this, &UFeelComfortMenu::HandleFlashLimiterChanged); }
	if (DefaultPresetButton) { DefaultPresetButton->OnClicked.AddUniqueDynamic(this, &UFeelComfortMenu::HandleDefaultPreset); }
	if (ReducedMotionPresetButton) { ReducedMotionPresetButton->OnClicked.AddUniqueDynamic(this, &UFeelComfortMenu::HandleReducedMotionPreset); }
	if (ReducedFlashingPresetButton) { ReducedFlashingPresetButton->OnClicked.AddUniqueDynamic(this, &UFeelComfortMenu::HandleReducedFlashingPreset); }
	if (NoHapticsPresetButton) { NoHapticsPresetButton->OnClicked.AddUniqueDynamic(this, &UFeelComfortMenu::HandleNoHapticsPreset); }
	if (TryButton) { TryButton->OnClicked.AddUniqueDynamic(this, &UFeelComfortMenu::HandleTry); }
	if (ResetButton) { ResetButton->OnClicked.AddUniqueDynamic(this, &UFeelComfortMenu::HandleReset); }
	if (CloseButton) { CloseButton->OnClicked.AddUniqueDynamic(this, &UFeelComfortMenu::HandleClose); }

	for (USlider* Slider : { MasterSlider.Get(), CameraShakeSlider.Get(), CameraMotionSlider.Get(), FlashesSlider.Get(), HitstopSlider.Get(), ScreenDistortionSlider.Get(), HapticsSlider.Get() })
	{
		if (Slider)
		{
			Slider->SetMinValue(0.0f);
			Slider->SetMaxValue(1.0f);
			Slider->SetStepSize(SliderStep);
		}
	}
	if (ZoomSpeedSlider)
	{
		// Like the other sliders, right allows more: the far right means no limit, further left means slower zooms.
		ZoomSpeedSlider->SetMinValue(0.0f);
		ZoomSpeedSlider->SetMaxValue(1.0f);
		ZoomSpeedSlider->SetStepSize(SliderStep);
	}

	// Previews must play in a paused menu too.
	if (UWorld* World = GetWorld())
	{
		if (UFeelSubsystem* Feel = World->GetSubsystem<UFeelSubsystem>())
		{
			Feel->AddPausedPlaybackRequest();
			bRequestedPausedPlayback = true;
		}
	}

	RefreshCloseHint();

	ButtonStates.Reset();
	auto TrackButton = [this](UButton* Button, bool bIsPreset, EFeelBuiltInComfortPreset Preset)
	{
		if (Button)
		{
			FButtonState& State = ButtonStates.AddDefaulted_GetRef();
			State.Button = Button;
			State.Original = Button->GetStyle();
			State.bIsPreset = bIsPreset;
			State.Preset = Preset;
		}
	};
	TrackButton(DefaultPresetButton, true, EFeelBuiltInComfortPreset::Default);
	TrackButton(ReducedMotionPresetButton, true, EFeelBuiltInComfortPreset::ReducedMotion);
	TrackButton(ReducedFlashingPresetButton, true, EFeelBuiltInComfortPreset::ReducedFlashing);
	TrackButton(NoHapticsPresetButton, true, EFeelBuiltInComfortPreset::NoHaptics);
	TrackButton(TryButton, false, EFeelBuiltInComfortPreset::Default);
	TrackButton(ResetButton, false, EFeelBuiltInComfortPreset::Default);
	TrackButton(CloseButton, false, EFeelBuiltInComfortPreset::Default);

	RefreshFromSettings();
	if (SaveStatusText)
	{
		SaveStatusText->SetText(FText::GetEmpty());
	}
}

void UFeelComfortMenu::NativeDestruct()
{
	if (bUnsaved)
	{
		SaveNow();
	}
	if (bRequestedPausedPlayback)
	{
		if (UWorld* World = GetWorld())
		{
			if (UFeelSubsystem* Feel = World->GetSubsystem<UFeelSubsystem>())
			{
				Feel->RemovePausedPlaybackRequest();
			}
		}
		bRequestedPausedPlayback = false;
	}
	Super::NativeDestruct();
}

void UFeelComfortMenu::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bUnsaved && FPlatformTime::Seconds() - LastChangeTime >= SaveDelay)
	{
		SaveNow();
	}
	UpdateSelection();
}

FReply UFeelComfortMenu::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (CloseKeys.Contains(Key))
	{
		CloseMenu();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UFeelComfortMenu::CloseMenu()
{
	if (bUnsaved)
	{
		SaveNow();
	}

	APlayerController* PlayerController = GetOwningPlayer();
	RemoveFromParent();

	if (bOwnsInputMode && PlayerController)
	{
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->bShowMouseCursor = bPreviousShowMouseCursor;
		if (bPausedGame)
		{
			UGameplayStatics::SetGamePaused(PlayerController, false);
		}
		bOwnsInputMode = false;
		bPausedGame = false;
	}
	OnClosed.Broadcast();
}

void UFeelComfortMenu::RefreshFromSettings()
{
	const UFeelComfortSubsystem* Comfort = GetComfort();
	if (!Comfort)
	{
		return;
	}
	const FFeelComfortScales& Scales = Comfort->GetComfortScalesRef();

	// SetValue and SetIsChecked do not call the change handlers, so refreshing never saves or previews.
	if (MasterSlider) { MasterSlider->SetValue(Scales.Master); }
	if (CameraShakeSlider) { CameraShakeSlider->SetValue(Scales.CameraShake); }
	if (CameraMotionSlider) { CameraMotionSlider->SetValue(Scales.CameraMotion); }
	if (FlashesSlider) { FlashesSlider->SetValue(Scales.Flashes); }
	if (HitstopSlider) { HitstopSlider->SetValue(Scales.HitstopAndSlowMo); }
	if (ScreenDistortionSlider) { ScreenDistortionSlider->SetValue(Scales.ScreenDistortion); }
	if (HapticsSlider) { HapticsSlider->SetValue(Scales.Haptics); }
	if (ZoomSpeedSlider)
	{
		const float Limit = Scales.MaxFieldOfViewChangePerSecond;
		ZoomSpeedSlider->SetValue(Limit <= 0.0f ? 1.0f : FMath::Clamp(Limit / MaxZoomSpeedLimit, 0.0f, 1.0f - SliderStep));
	}
	if (CameraRollCheckBox) { CameraRollCheckBox->SetIsChecked(Scales.bAllowCameraRoll); }
	if (FlashLimiterCheckBox) { FlashLimiterCheckBox->SetIsChecked(Scales.bLimitFlashes); }
	UpdateValueTexts(Scales);
}

void UFeelComfortMenu::RefreshCloseHint()
{
	if (!CloseHintText)
	{
		return;
	}

	auto Label = [](const FKey& Key)
	{
		if (Key == EKeys::Escape) { return LOCTEXT("KeyEsc", "Esc"); }
		if (Key == EKeys::Gamepad_FaceButton_Right) { return LOCTEXT("KeyB", "B / Circle"); }
		if (Key == EKeys::Gamepad_FaceButton_Bottom) { return LOCTEXT("KeyA", "A / Cross"); }
		if (Key == EKeys::Gamepad_Special_Right) { return LOCTEXT("KeyMenu", "Menu / Options"); }
		if (Key == EKeys::Gamepad_Special_Left) { return LOCTEXT("KeyView", "View / Share"); }
		return Key.GetDisplayName(false);
	};

	TArray<FText> KeyboardKeys;
	const FKey* Gamepad = nullptr;
	for (const FKey& Key : CloseKeys)
	{
		if (!Key.IsValid())
		{
			continue;
		}
		if (Key.IsGamepadKey())
		{
			Gamepad = Gamepad ? Gamepad : &Key;
		}
		else
		{
			KeyboardKeys.Add(Label(Key));
		}
	}

	FText Keyboard;
	if (KeyboardKeys.Num() == 1)
	{
		Keyboard = KeyboardKeys[0];
	}
	else if (KeyboardKeys.Num() > 1)
	{
		const FText Last = KeyboardKeys.Pop();
		Keyboard = FText::Format(LOCTEXT("KeysOr", "{0} or {1}"), FText::Join(LOCTEXT("KeysComma", ", "), KeyboardKeys), Last);
	}

	FText Hint;
	if (!Keyboard.IsEmpty() && Gamepad)
	{
		Hint = FText::Format(LOCTEXT("CloseHintBoth", "{0} to close, {1} on a controller"), Keyboard, Label(*Gamepad));
	}
	else if (!Keyboard.IsEmpty())
	{
		Hint = FText::Format(LOCTEXT("CloseHintKeyboard", "{0} to close"), Keyboard);
	}
	else if (Gamepad)
	{
		Hint = FText::Format(LOCTEXT("CloseHintGamepad", "{0} to close"), Label(*Gamepad));
	}
	CloseHintText->SetText(Hint);
}

void UFeelComfortMenu::PlayTry()
{
	PlayRecipe(TryRecipe);
}

UFeelComfortSubsystem* UFeelComfortMenu::GetComfort() const
{
	return UFeelBlueprintLibrary::GetFeelComfort(GetOwningPlayer());
}

void UFeelComfortMenu::ChangeScales(EFeelComfortMenuRow Row, TFunctionRef<void(FFeelComfortScales&)> Change)
{
	UFeelComfortSubsystem* Comfort = GetComfort();
	if (!Comfort)
	{
		return;
	}
	FFeelComfortScales Scales = Comfort->GetComfortScales();
	Change(Scales);
	Comfort->SetComfortScalesWithoutSaving(Scales);
	UpdateValueTexts(Comfort->GetComfortScalesRef());

	bUnsaved = true;
	LastChangeTime = FPlatformTime::Seconds();
	if (SaveStatusText && GetDefault<UFeelSettings>()->bAutoSaveComfort)
	{
		SaveStatusText->SetText(UnsavedText);
	}
	PlayPreview(Row);
}

void UFeelComfortMenu::ApplyPreset(EFeelBuiltInComfortPreset Preset)
{
	if (UFeelComfortSubsystem* Comfort = GetComfort())
	{
		Comfort->ApplyComfortPreset(Preset);
		bUnsaved = false;
		RefreshFromSettings();
		if (SaveStatusText && GetDefault<UFeelSettings>()->bAutoSaveComfort)
		{
			SaveStatusText->SetText(SavedText);
		}
	}
}

void UFeelComfortMenu::SaveNow()
{
	bUnsaved = false;
	if (!GetDefault<UFeelSettings>()->bAutoSaveComfort)
	{
		return;
	}
	UFeelComfortSubsystem* Comfort = GetComfort();
	if (Comfort && Comfort->SaveComfortSettings() && SaveStatusText)
	{
		SaveStatusText->SetText(SavedText);
	}
}

void UFeelComfortMenu::UpdateValueTexts(const FFeelComfortScales& Scales)
{
	auto SetPercent = [](UTextBlock* Text, float Value)
	{
		if (Text)
		{
			Text->SetText(FText::Format(LOCTEXT("Percent", "{0}%"), FText::AsNumber(FMath::RoundToInt(Value * 100.0f))));
		}
	};
	SetPercent(MasterValue, Scales.Master);
	SetPercent(CameraShakeValue, Scales.CameraShake);
	SetPercent(CameraMotionValue, Scales.CameraMotion);
	SetPercent(FlashesValue, Scales.Flashes);
	SetPercent(HitstopValue, Scales.HitstopAndSlowMo);
	SetPercent(ScreenDistortionValue, Scales.ScreenDistortion);
	SetPercent(HapticsValue, Scales.Haptics);
	if (ZoomSpeedValue)
	{
		ZoomSpeedValue->SetText(Scales.MaxFieldOfViewChangePerSecond <= 0.0f
			? NoLimitText
			: FText::Format(LOCTEXT("DegreesPerSecond", "{0}°/s"), FText::AsNumber(FMath::RoundToInt(Scales.MaxFieldOfViewChangePerSecond))));
	}
}

EFeelComfortMenuRow UFeelComfortMenu::FindSelectedRow(bool& bOutFocused) const
{
	const APlayerController* PlayerController = GetOwningPlayer();
	auto HasFocus = [PlayerController](const UWidget* Widget)
	{
		return Widget && (Widget->HasKeyboardFocus() || (PlayerController && Widget->HasUserFocus(const_cast<APlayerController*>(PlayerController))));
	};

	struct FRowControls
	{
		EFeelComfortMenuRow Row;
		const UWidget* Control;
		const UBorder* Border;
	};
	const FRowControls Rows[] = {
		{ EFeelComfortMenuRow::Master, MasterSlider, MasterRow },
		{ EFeelComfortMenuRow::CameraShake, CameraShakeSlider, CameraShakeRow },
		{ EFeelComfortMenuRow::CameraMotion, CameraMotionSlider, CameraMotionRow },
		{ EFeelComfortMenuRow::Flashes, FlashesSlider, FlashesRow },
		{ EFeelComfortMenuRow::HitstopAndSlowMo, HitstopSlider, HitstopRow },
		{ EFeelComfortMenuRow::ScreenDistortion, ScreenDistortionSlider, ScreenDistortionRow },
		{ EFeelComfortMenuRow::Haptics, HapticsSlider, HapticsRow },
		{ EFeelComfortMenuRow::CameraRoll, CameraRollCheckBox, CameraRollRow },
		{ EFeelComfortMenuRow::FieldOfViewSpeed, ZoomSpeedSlider, ZoomSpeedRow },
		{ EFeelComfortMenuRow::FlashLimiter, FlashLimiterCheckBox, FlashLimiterRow },
	};
	for (const FRowControls& Entry : Rows)
	{
		if (HasFocus(Entry.Control))
		{
			bOutFocused = true;
			return Entry.Row;
		}
	}
	for (const UButton* Button : { DefaultPresetButton.Get(), ReducedMotionPresetButton.Get(), ReducedFlashingPresetButton.Get(), NoHapticsPresetButton.Get() })
	{
		if (HasFocus(Button))
		{
			bOutFocused = true;
			return EFeelComfortMenuRow::Presets;
		}
	}
	if (HasFocus(TryButton))
	{
		bOutFocused = true;
		return EFeelComfortMenuRow::Try;
	}

	bOutFocused = false;
	for (const FRowControls& Entry : Rows)
	{
		if (Entry.Border && Entry.Border->IsHovered())
		{
			return Entry.Row;
		}
	}
	return EFeelComfortMenuRow::None;
}

void UFeelComfortMenu::UpdateSelection()
{
	bool bFocused = false;
	const EFeelComfortMenuRow Selected = FindSelectedRow(bFocused);

	const TPair<EFeelComfortMenuRow, UBorder*> Borders[] = {
		{ EFeelComfortMenuRow::Master, MasterRow },
		{ EFeelComfortMenuRow::CameraShake, CameraShakeRow },
		{ EFeelComfortMenuRow::CameraMotion, CameraMotionRow },
		{ EFeelComfortMenuRow::Flashes, FlashesRow },
		{ EFeelComfortMenuRow::HitstopAndSlowMo, HitstopRow },
		{ EFeelComfortMenuRow::ScreenDistortion, ScreenDistortionRow },
		{ EFeelComfortMenuRow::Haptics, HapticsRow },
		{ EFeelComfortMenuRow::CameraRoll, CameraRollRow },
		{ EFeelComfortMenuRow::FieldOfViewSpeed, ZoomSpeedRow },
		{ EFeelComfortMenuRow::FlashLimiter, FlashLimiterRow },
	};
	for (const TPair<EFeelComfortMenuRow, UBorder*>& Entry : Borders)
	{
		if (Entry.Value)
		{
			const bool bSelected = Entry.Key == Selected;
			Entry.Value->SetBrushColor(bSelected ? (bFocused ? SelectedRowColor : HoveredRowColor) : RowColor);
		}
	}

	UpdateButtons();

	if (DescriptionText && Selected != EFeelComfortMenuRow::None && Selected != ShownDescriptionRow)
	{
		const FText* Description = Descriptions.Find(Selected);
		DescriptionText->SetText(Description ? *Description : FText::GetEmpty());
		ShownDescriptionRow = Selected;
	}
}

void UFeelComfortMenu::UpdateButtons()
{
	const UFeelComfortSubsystem* Comfort = GetComfort();
	const UFeelSettings* Settings = GetDefault<UFeelSettings>();
	APlayerController* PlayerController = GetOwningPlayer();
	auto Same = [](const FFeelComfortScales& A, const FFeelComfortScales& B)
	{
		constexpr float Tolerance = 0.001f;
		return FMath::IsNearlyEqual(A.Master, B.Master, Tolerance) && FMath::IsNearlyEqual(A.CameraShake, B.CameraShake, Tolerance)
			&& FMath::IsNearlyEqual(A.CameraMotion, B.CameraMotion, Tolerance) && FMath::IsNearlyEqual(A.Flashes, B.Flashes, Tolerance)
			&& FMath::IsNearlyEqual(A.HitstopAndSlowMo, B.HitstopAndSlowMo, Tolerance) && FMath::IsNearlyEqual(A.ScreenDistortion, B.ScreenDistortion, Tolerance)
			&& FMath::IsNearlyEqual(A.Haptics, B.Haptics, Tolerance) && A.bAllowCameraRoll == B.bAllowCameraRoll && A.bLimitFlashes == B.bLimitFlashes
			&& FMath::IsNearlyEqual(A.MaxFieldOfViewChangePerSecond, B.MaxFieldOfViewChangePerSecond, 0.5f);
	};

	for (FButtonState& State : ButtonStates)
	{
		UButton* Button = State.Button.Get();
		if (!Button)
		{
			continue;
		}
		const bool bFocused = Button->HasKeyboardFocus() || (PlayerController && Button->HasUserFocus(PlayerController));
		const bool bActive = State.bIsPreset && Comfort && Same(Comfort->GetComfortScalesRef(), Settings->GetPresetScales(State.Preset));
		const uint8 Wanted = bFocused ? 1 : (bActive ? 2 : 0);
		if (Wanted != State.Shown)
		{
			FButtonStyle Style = State.Original;
			if (Wanted == 1)
			{
				Style.SetNormal(State.Original.Hovered);
			}
			else if (Wanted == 2)
			{
				Style.SetNormal(State.Original.Pressed).SetHovered(State.Original.Pressed);
			}
			Button->SetStyle(Style);
			State.Shown = Wanted;
		}
	}
}

void UFeelComfortMenu::PlayPreview(EFeelComfortMenuRow Row)
{
	if (!bLivePreview)
	{
		return;
	}
	const double Now = FPlatformTime::Seconds();
	double& Last = LastPreviewTimes.FindOrAdd(Row, -1000.0);
	if (Now - Last < PreviewInterval)
	{
		return;
	}
	Last = Now;

	if (Row == EFeelComfortMenuRow::Master)
	{
		PlayRecipe(TryRecipe);
	}
	else if (const TObjectPtr<UFeelRecipe>* Recipe = PreviewRecipes.Find(Row))
	{
		PlayRecipe(*Recipe);
	}
}

void UFeelComfortMenu::PlayRecipe(UFeelRecipe* Recipe)
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (!Recipe || !PlayerController)
	{
		return;
	}
	// Played on the menu's own player, so split-screen previews reach the right screen.
	FFeelTarget Target;
	if (APawn* Pawn = PlayerController->GetPawn())
	{
		Target = FFeelTarget::FromActor(Pawn);
	}
	else if (const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
	{
		Target = FFeelTarget::FromLocalPlayerCamera(LocalPlayer->GetLocalPlayerIndex());
	}
	UFeelBlueprintLibrary::PlayFeel(this, Recipe, Target, 1.0f);
}

void UFeelComfortMenu::HandleMasterChanged(float Value) { ChangeScales(EFeelComfortMenuRow::Master, [Value](FFeelComfortScales& S) { S.Master = Value; }); }
void UFeelComfortMenu::HandleCameraShakeChanged(float Value) { ChangeScales(EFeelComfortMenuRow::CameraShake, [Value](FFeelComfortScales& S) { S.CameraShake = Value; }); }
void UFeelComfortMenu::HandleCameraMotionChanged(float Value) { ChangeScales(EFeelComfortMenuRow::CameraMotion, [Value](FFeelComfortScales& S) { S.CameraMotion = Value; }); }
void UFeelComfortMenu::HandleFlashesChanged(float Value) { ChangeScales(EFeelComfortMenuRow::Flashes, [Value](FFeelComfortScales& S) { S.Flashes = Value; }); }
void UFeelComfortMenu::HandleHitstopChanged(float Value) { ChangeScales(EFeelComfortMenuRow::HitstopAndSlowMo, [Value](FFeelComfortScales& S) { S.HitstopAndSlowMo = Value; }); }
void UFeelComfortMenu::HandleScreenDistortionChanged(float Value) { ChangeScales(EFeelComfortMenuRow::ScreenDistortion, [Value](FFeelComfortScales& S) { S.ScreenDistortion = Value; }); }
void UFeelComfortMenu::HandleHapticsChanged(float Value) { ChangeScales(EFeelComfortMenuRow::Haptics, [Value](FFeelComfortScales& S) { S.Haptics = Value; }); }

void UFeelComfortMenu::HandleZoomSpeedChanged(float Value)
{
	const float Limit = Value >= 1.0f - KINDA_SMALL_NUMBER ? 0.0f : FMath::Max(10.0f, FMath::RoundToFloat(Value * MaxZoomSpeedLimit / 10.0f) * 10.0f);
	ChangeScales(EFeelComfortMenuRow::FieldOfViewSpeed, [Limit](FFeelComfortScales& S) { S.MaxFieldOfViewChangePerSecond = Limit; });
}

void UFeelComfortMenu::HandleCameraRollChanged(bool bIsChecked) { ChangeScales(EFeelComfortMenuRow::CameraRoll, [bIsChecked](FFeelComfortScales& S) { S.bAllowCameraRoll = bIsChecked; }); }
void UFeelComfortMenu::HandleFlashLimiterChanged(bool bIsChecked) { ChangeScales(EFeelComfortMenuRow::FlashLimiter, [bIsChecked](FFeelComfortScales& S) { S.bLimitFlashes = bIsChecked; }); }
void UFeelComfortMenu::HandleDefaultPreset() { ApplyPreset(EFeelBuiltInComfortPreset::Default); }
void UFeelComfortMenu::HandleReducedMotionPreset() { ApplyPreset(EFeelBuiltInComfortPreset::ReducedMotion); }
void UFeelComfortMenu::HandleReducedFlashingPreset() { ApplyPreset(EFeelBuiltInComfortPreset::ReducedFlashing); }
void UFeelComfortMenu::HandleNoHapticsPreset() { ApplyPreset(EFeelBuiltInComfortPreset::NoHaptics); }
void UFeelComfortMenu::HandleTry() { PlayTry(); }
void UFeelComfortMenu::HandleReset() { ApplyPreset(EFeelBuiltInComfortPreset::Default); }
void UFeelComfortMenu::HandleClose() { CloseMenu(); }

#undef LOCTEXT_NAMESPACE
